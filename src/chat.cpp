#include "header.h"
#include "utils.h"
#include "openai.h"
#include "server.h"
#include "parse_json.h"
#include "chat_header.h"
#include "../gpt4all-backend/llmodel_c.h"



//////////////////////////////////////////////////////////////////////////
////////////                  MAIN PROGRAM                    ////////////
//////////////////////////////////////////////////////////////////////////


int main(int argc, char* argv[]) {


    ConsoleState con_st;
    con_st.use_color = true;
    set_console_color(con_st, DEFAULT);

    set_console_color(con_st, PROMPT);
    set_console_color(con_st, BOLD);
    std::cout << APPNAME;
    set_console_color(con_st, DEFAULT);
    set_console_color(con_st, PROMPT);
    std::cout << " (v. " << VERSION << ")";
    set_console_color(con_st, DEFAULT);
    std::cout << "" << std::endl;
    check_avx_support_at_startup();

    chatParams params;
    //convert the default model path into Windows format if on WIN32
    #ifdef _WIN32
        std::filesystem::path p(params.model);
        params.model = p.make_preferred().string();
    #endif
 
    //get all parameters from cli arguments or json
    parse_params(argc, argv, params);

    //if model is one of the openai-models, set openai-flag
    if (params.model == "gpt-3.5-turbo" || params.model == "gpt-4") {
        params.openai = true;
    }
    
    //Create a prompt_context and copy all params from chatParams to prompt_context
    llmodel_prompt_context prompt_context = {
     .logits = params.logits,
     .logits_size = params.logits_size,
     .tokens = params.tokens,
     .tokens_size = params.tokens_size,
     .n_past = params.n_past,
     .n_ctx = params.n_ctx,
     .n_predict = params.n_predict,
     .top_k = params.top_k,
     .top_p = params.top_p,
     .temp = params.temp,
     .n_batch = params.n_batch,
     .repeat_penalty = params.repeat_penalty,  
     .repeat_last_n = params.repeat_last_n,
     .context_erase = params.context_erase,
    }; 
 
    //load the model
    llmodel_model model = nullptr;
    if (!params.openai) {
        model = load_model_with_loading_animation(params);
        std::cout << std::endl;
    }

    set_console_color(con_st, PROMPT);
    std::cout << params.prompt.c_str() << std::endl;
    set_console_color(con_st, DEFAULT);


    //default prompt template, should work with most instruction-type models
    std::tuple<std::string, std::string, std::string> prompt_template;
    prompt_template = read_prompt_template_file(params.load_template);



    //////////////////////////////////////////////////////////////////////////
    ////////////         PROMPT TEXT AND GET RESPONSE             ////////////
    //////////////////////////////////////////////////////////////////////////

    //if server-mode is on, we switch to server.h loop instead 
    if (params.server) {
        server(params, con_st, model, prompt_context, prompt_template);
        llmodel_model_destroy(model);
        return 0;
    }

    // Main chat loop.
    std::string input = "";
    if (!params.no_interactive) {
        //Interactive mode. Else get prompt from input or from params.
        input = get_input(con_st, input, params, prompt_context, model);
        if (params.openai) { process_openai_input(input, params, model, prompt_context, prompt_template); }
        else { process_chat_input(input, params, model, prompt_context, prompt_template); }
        //Interactive and continuous mode. Get prompt from input.
        while (!params.run_once) {
            answer = "";
            input = get_input(con_st, input, params, prompt_context, model);
        if (params.openai) { process_openai_input(input, params, model, prompt_context, prompt_template); }
        else { process_chat_input(input, params, model, prompt_context, prompt_template); }

        }
    //No-interactive mode. Get the answer once from prompt and print it.
    } else {
        if (params.openai) { process_openai_input(input, params, model, prompt_context, prompt_template); }
        else { process_chat_input(input, params, model, prompt_context, prompt_template); }
        std::cout << std::endl;
    }


    set_console_color(con_st, DEFAULT);
    llmodel_model_destroy(model);
    return 0;
}
