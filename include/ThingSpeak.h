#pragma once

#include "esp_http_client.h"
#include "esp_log.h"

#include <algorithm>
#include <sstream>
#include <string>
#include <sys/time.h>
#include <vector>

#include "json11.hpp"

#define THINGSPEAK_FORMAT_JSON 0

class ThingSpeak {
private:
  constexpr static const char *host = "api.thingspeak.com";
  constexpr static const char *url_base = "https://api.thingspeak.com";

public:
  ThingSpeak(const char *channel_id, const char *api_key)
      : channel_id(channel_id), api_key(api_key) {
    url_json =
        (std::string)url_base + "/channels/" + channel_id + "/bulk_update.json";
    url_plain =
        (std::string)url_base + "/channels/" + channel_id + "/bulk_update";
    url_single = (std::string)url_base + "/update?api_key=" + api_key;
  }

  struct Feed {
    std::vector<std::pair<int, float>> fields;
    std::string created_at;

    Feed() {}
    Feed(std::vector<std::pair<int, float>> &fields,
         std::string created_at = getTimeISO8601())
        : fields(fields), created_at(created_at) {}
  };

  bool post(const std::vector<Feed> &feeds) {
#if THINGSPEAK_FORMAT_JSON
    return postMulti(url_json, "application/json", generateJson(feeds));
#else
    return postMulti(url_plain, "application/x-www-form-urlencoded",
                     generatePlain(feeds));
#endif
  }
  bool post(const Feed &feed, int timeout_ms = 2000) {
    std::string url = url_single;
    url += "&created_at=" + feed.created_at;
    for (auto field : feed.fields)
      url += "&field" + std::to_string(field.first) + "=" +
             std::to_string(field.second);
    return postSingle(url);
  }

  static std::string getTimeISO8601() {
    timeval curTime;
    time_t now;
    time(&now);
    gettimeofday(&curTime, NULL);
    int milli = curTime.tv_usec / 1000;
    char buf[sizeof "2011-10-08T07:07:09"];
    strftime(buf, sizeof buf, "%FT%T", gmtime(&now));
    sprintf(buf, "%s.%dZ", buf, milli);
    return buf;
  }

private:
  const char *tag = "ThingSpeak";
  const char *channel_id;
  const char *api_key;
  std::string url_json;
  std::string url_plain;
  std::string url_single;

  std::string generateJson(const std::vector<Feed> &feeds) {
    json11::Json::object obj;
    obj["write_api_key"] = api_key;
    json11::Json::array updates_ary;
    for (const auto &feed : feeds) {
      json11::Json::object obj_field;
      obj_field["created_at"] = feed.created_at;
      for (const auto &field : feed.fields) {
        std::stringstream field_str;
        field_str << "field" << field.first;
        obj_field[field_str.str()] = field.second;
      }
      updates_ary.push_back(json11::Json(obj_field));
    }
    obj["updates"] = updates_ary;
    return json11::Json(obj).dump();
  }

  std::string generatePlain(const std::vector<Feed> &feeds) {
    std::stringstream data;
    data << "write_api_key=" << api_key;
    data << "&time_format=absolute";
    data << "&updates=";
    for (const auto &feed : feeds) {
      // created_at
      data << feed.created_at << ",";
      // field1, ..., field8
      for (int ch = 1; ch <= 8; ch++) {
        auto it = std::find_if(feed.fields.begin(), feed.fields.end(),
                               [&ch](auto f) { return f.first == ch; });
        if (it != feed.fields.end())
          data << (*it).second;
        data << ",";
      }
      // lattitude, longitude, elevation
      for (int i = 0; i < 3; ++i)
        data << ",";
      // status
      data << "|";
    }
    return data.str();
  }

  const std::string generateJson(const Feed &feed) {
    json11::Json::object obj;
    obj["api_key"] = api_key;
    obj["created_at"] = feed.created_at;
    for (auto field : feed.fields) {
      std::stringstream field_str;
      field_str << "field" << field.first;
      obj[field_str.str()] = field.second;
    }
    return json11::Json(obj).dump();
  }

  bool postSingle(const std::string &url, int timeout_ms = 2000) {
    static esp_http_client_config_t config;
    config.method = HTTP_METHOD_GET;
    config.url = url.c_str();
    config.timeout_ms = timeout_ms;
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Host", host);
    ESP_LOGI(tag, "HTTP starting...");
    esp_http_client_perform(client);
    ESP_LOGI(tag, "content: %s", url.c_str());
    int status_code = esp_http_client_get_status_code(client);
    ESP_LOGI(tag, "status code: %d", esp_http_client_get_status_code(client));
    esp_http_client_cleanup(client);
    return status_code / 100 == 2;
  }
  bool postMulti(const std::string &url, const std::string &content_type,
                 const std::string &content, int timeout_ms = 2000) {
    static esp_http_client_config_t config;
    config.method = HTTP_METHOD_POST;
    config.url = url.c_str();
    config.timeout_ms = timeout_ms;
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Host", host);
    esp_http_client_set_header(client, "Content-Type", content_type.c_str());
    esp_http_client_set_post_field(client, content.c_str(), content.size());
    ESP_LOGI(tag, "HTTP starting...");
    esp_http_client_perform(client);
    ESP_LOGI(tag, "content: %s", content.c_str());
    int status_code = esp_http_client_get_status_code(client);
    ESP_LOGI(tag, "status code: %d", esp_http_client_get_status_code(client));
    esp_http_client_cleanup(client);
    return status_code / 100 == 2;
  }
};