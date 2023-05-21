#pragma once

#ifndef OPENAI_H
#define OPENAI_H

#include "header.h" 
#include "utils.h" 

namespace openai {

class OpenAIApi {
public:
  explicit OpenAIApi(const std::string& token) : _token(token) {}

  nlohmann::json post(const std::string& path, const nlohmann::json& json_data) {
    httplib::Client cli("https://api.openai.com");
    httplib::Headers headers = {
      {"Authorization", "Bearer " + _token},
      {"Content-Type", "application/json"}
    };
    auto res = cli.Post(path.c_str(), headers, json_data.dump(), "application/json");
    if (res && res->status == 200) {
      return nlohmann::json::parse(res->body);
    } else {
      // Handle the error
    }
  }

  nlohmann::json get(const std::string& path) {
    httplib::Client cli("https://api.openai.com");
    httplib::Headers headers = {
      {"Authorization", "Bearer " + _token}
    };
    auto res = cli.Get(path.c_str(), headers);
    if (res && res->status == 200) {
      return nlohmann::json::parse(res->body);
    } else {
      // Handle the error
    }
  }

private:
  std::string _token;
};

} // namespace openai


#endif
