#pragma once

#ifndef SERVER_H
#define SERVER_H

#include "header.h" 
#include "utils.h" 
#include "openai.h" 
#include "chat_header.h"
#include "../gpt4all-backend/llmodel_c.h"


int server(chatParams &params, ConsoleState &con_st, llmodel_model &model, llmodel_prompt_context &prompt_context, std::tuple<std::string, std::string, std::string> &prompt_template) {
  
  using namespace httplib;
  using json = nlohmann::json;
 
  Server *svr;

  if (params.ssl_server) {
      //SSLServer svr = server(params.ssl_certificate, params.ssl_certificate_key);
      svr = new SSLServer(params.ssl_certificate.c_str(), params.ssl_certificate_key.c_str());
  } else {
      svr = new Server();
  }

  //we get a server POST request
  svr->Post("/v1/completions", [&params,&con_st,&model,&prompt_context,&prompt_template](const Request& req, Response& res) {

    std::string current_model = params.model;
    // Parse request body as JSON
    auto req_json = json::parse(req.body);
    std::cout << req_json.dump() << std::endl;

    //store the parameters from the Request into chatParams and prompt_context
    params.model = req_json["model"];
    params.prompt = req_json["prompt"];
    params.temp = req_json["temperature"];
    params.n_predict = req_json["max_tokens"];
    params.top_p = req_json["top_p"];
    params.n_batch = req_json["n"];
 
    prompt_context.temp = req_json["temperature"];
    prompt_context.n_predict = req_json["max_tokens"];
    prompt_context.top_p = req_json["top_p"];
    prompt_context.n_batch = req_json["n"];
 
    //if the model name in the Request has changed, we need to load a new one
    if (params.model != current_model) {
      
      if (params.model == "gpt-3.5-turbo" || params.model == "gpt-4") {
        params.openai = true;
      } else {
        params.openai = false;
      }
      if (params.openai) {
          llmodel_model_destroy(model);
          params.openai = true;
          model = nullptr;
      } else {
          llmodel_model_destroy(model);
          model = load_model_with_loading_animation(params);
          std::cout << std::endl;
      }
    }


    //main response loop
    std::string input = "";
    if (params.openai) { process_openai_input(input, params, model, prompt_context, prompt_template); }
    else { process_chat_input(input, params, model, prompt_context, prompt_template); }
    std::cout << std::endl;

    


    set_console_color(con_st, DEFAULT);

    // Build the response JSON
    json response = {
      {"choices", {
        {
          {"finish_reason", "stop"},
          {"index", 0},
          {"logprobs", nullptr},
          {"text", answer.c_str()}
        }
      }},
      {"created", unixtime()},
      {"id", "chatcmpl"},
      {"model", params.model},
      {"object", "text_completion"},
      {"usage", {
        {"completion_tokens", answer_tokens_length},
        {"prompt_tokens", prompt_tokens_length},
        {"total_tokens", prompt_tokens_length+answer_tokens_length}
      }}
    };

    // Clear the answer as the answer has now been printed
    answer = "";
    // Set response content type and body
    res.set_content(response.dump(), "application/json");
  });

  //listen port 5891 (1985 = 1984 + 1 backwards)
  //svr->listen("localhost", 5891);
  svr->listen("0.0.0.0", 5891);
  llmodel_model_destroy(model);
}

#endif


