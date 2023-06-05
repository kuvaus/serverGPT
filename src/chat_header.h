#pragma once

#ifndef CHAT_HEADER_H
#define CHAT_HEADER_H

#include "header.h"
#include "utils.h"
#include "openai.h"
#include "../gpt4all-backend/llmodel_c.h"


//////////////////////////////////////////////////////////////////////////
////////////                OPENAI CHAT INPUT                 ////////////
//////////////////////////////////////////////////////////////////////////
void process_openai_input(std::string& input, chatParams& params, llmodel_model& model, llmodel_prompt_context& prompt_context, std::tuple<std::string, std::string, std::string> prompt_template) {
    
    prompt_tokens_length = 0; //new prompt
    answer_tokens_length = 0; //new answer

    std::string prompt = params.prompt != "" ? params.prompt + " " + input : input;
    std::string full_prompt = std::get<0>(prompt_template) + std::get<1>(prompt_template) + prompt + std::get<2>(prompt_template);

    //load chat log
    if (params.load_log != "") {
        full_prompt = std::get<0>(prompt_template) + read_chat_log(params.load_log) + std::get<1>(prompt_template) + prompt + std::get<2>(prompt_template);
        //set it to empty again so that we run this only once in the beginning.
        params.load_log == "";
    }
    
    std::future<void> future;

    if (params.use_animation) { stop_display = false; future = std::async(std::launch::async, display_frames); }
        openai::OpenAIApi api(params.openai_api_key);
        // Send GPT-3 query
          nlohmann::json j = {
            {"model", params.model},
            {"messages", {{{"role", "user"}, {"content", prompt}}}},
            {"max_tokens", params.n_predict},
            {"temperature", params.temp},
            {"top_p", params.top_p},
            {"n", 1}
        };
        nlohmann::json completion = api.post("/v1/chat/completions", j);
        //std::cout << completion.dump() << std::endl;
        answer = completion["choices"][0]["message"]["content"];
        if (params.use_animation){ stop_display = true; future.wait(); stop_display = false; }
        std::cout << answer << std::flush;

        answer_tokens_length = completion["usage"]["completion_tokens"];
        prompt_tokens_length = completion["usage"]["prompt_tokens"];
}



//////////////////////////////////////////////////////////////////////////
////////////                 LOCAL CHAT INPUT                 ////////////
//////////////////////////////////////////////////////////////////////////
void process_chat_input(std::string& input, chatParams& params, llmodel_model& model, llmodel_prompt_context& prompt_context, std::tuple<std::string, std::string, std::string> prompt_template) {

    prompt_tokens_length = 0; //new prompt
    answer_tokens_length = 0; //new answer

    //////////////////////////////////////////////////////////////////////////
    ////////////            PROMPT LAMBDA FUNCTIONS               ////////////
    //////////////////////////////////////////////////////////////////////////
    auto prompt_callback = [](int32_t token_id)  {
	    // You can handle prompt here if needed
        prompt_tokens_length++;
	    return true;
	};

    auto response_callback = [](int32_t token_id, const char *responsechars) {
    
        if (!(responsechars == nullptr || responsechars[0] == '\0')) {
	    // stop the animation, printing response
        if (stop_display == false) {
	        stop_display = true;
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            std::cerr << "\r" << " " << std::flush;
            std::cerr << "\r" << std::flush;
        }
            
			std::cout << responsechars << std::flush;
	        answer += responsechars;
            answer_tokens_length++;
	    }
	            
	    return true;
	};
	
    auto recalculate_callback = [](bool is_recalculating) {
        // You can handle recalculation requests here if needed
        return is_recalculating;
    };



    std::future<void> future;
    std::string prompt = params.prompt != "" ? params.prompt + " " + input : input;
    std::string full_prompt = std::get<0>(prompt_template) + std::get<1>(prompt_template) + prompt + std::get<2>(prompt_template);

    //load chat log
    if (params.load_log != "") {
        full_prompt = std::get<0>(prompt_template) + read_chat_log(params.load_log) + std::get<1>(prompt_template) + prompt + std::get<2>(prompt_template);
        //set it to empty again so that we run this only once in the beginning.
        params.load_log == "";
    }

    if (params.use_animation) { stop_display = false; future = std::async(std::launch::async, display_frames); }
    llmodel_prompt(model, full_prompt.c_str(), prompt_callback, response_callback, recalculate_callback, &prompt_context); 
    if (params.use_animation) { stop_display = true; future.wait(); stop_display = false; }
    
    if (params.save_log != "") {
        save_chat_log(params.save_log, full_prompt.c_str(), answer.c_str());
    }

}


//////////////////////////////////////////////////////////////////////////
////////////                 LOAD THE MODEL                   ////////////
////////////////////////////////////////////////////////////////////////// 
llmodel_model load_model_with_loading_animation(chatParams &params){

    //animation
    std::future<void> future;
    
    stop_display = true;
    if(params.use_animation) {stop_display = false; future = std::async(std::launch::async, display_loading);}
    //handle stderr for now
    //this is just to prevent printing unnecessary details during model loading.
    int stderr_copy = dup(fileno(stderr));
    #ifdef _WIN32
        std::freopen("NUL", "w", stderr);
    #else
        std::freopen("/dev/null", "w", stderr);
    #endif

    llmodel_model model = llmodel_model_create(params.model.c_str());
    std::cout << "\r" << APPNAME << ": loading " << params.model.c_str()  << std::endl;
    
    //bring back stderr for now
    dup2(stderr_copy, fileno(stderr));
    close(stderr_copy);
    
    //check if model is loaded
    auto check_model = llmodel_loadModel(model, params.model.c_str());

    if (check_model == false) {
        if(params.use_animation) {
            stop_display = true;
            future.wait();
            stop_display= false;
        }

        std::cerr << "Error loading: " << params.model.c_str() << std::endl;
        std::cout << "Press any key to exit..." << std::endl;
        std::cin.get();
        return 0;
    } else {
        if(params.use_animation) {
            stop_display = true;
            future.wait();
        }
        std::cout << "\r" << APPNAME << ": done loading!" << std::flush;   
    }
    llmodel_setThreadCount(model, params.n_threads);
    return model;
}

#endif