#include "gramconfig/configuration.hpp"

#include <tinyxml2.h>

#include <chrono>
#include <filesystem>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "gramconfig/configurable.hpp"

namespace gramConfig {

Configuration::Configuration(const std::string &xml_filename)
    : config_filename_(xml_filename), config_document_(), file_monitor_running_(true) {
  tinyxml2::XMLError xml_error = config_document_.LoadFile(xml_filename.c_str());
  if (xml_error != tinyxml2::XML_SUCCESS) {
    throw new std::runtime_error("Unable to open configuration file");
  }

  // Verify the file is a somewhat valid XML
  tinyxml2::XMLNode *root_node = config_document_.FirstChild();
  if (root_node == nullptr) {
    throw new std::runtime_error("Unable to read XML format");
  }
}

Configuration::~Configuration(void) {}

std::shared_ptr<void> Configuration::AddListener(const NameList &configurations, Callback callback) {
  // Create a shared pointer for the callback function, cannibalising callback variable in the process
  auto callback_pointer = std::make_shared<Callback>(std::move(callback));

  // Add the new callback pointer to the callback list along with the provided configuration
  // Ensure that nothing has currently locked the callbacks vector before writing to it
  const std::lock_guard<std::mutex> lock(callbacks_mutex_);
  callbacks_.push_back(std::pair<NameList, std::weak_ptr<Callback>>(std::move(configurations), callback_pointer));

  // Return the callback pointer to the caller to use as a keepalive token
  return callback_pointer;
}

std::shared_ptr<void> Configuration::AddListener(const NameList &configurations, Configurable &callback) {
  return AddListener(configurations, [&](ValueList config) { callback.ConfigurationUpdate(config); });
}

void Configuration::UpdateListeners(void) {
  // Reload the configuration file as it's probably changed now
  tinyxml2::XMLError xml_error = config_document_.LoadFile(config_filename_.c_str());
  if (xml_error != tinyxml2::XML_SUCCESS) {
    throw new std::runtime_error("Unable to open configuration file");
  }

  // Iterate through the callback list and remove any non-existent callbacks
  const std::lock_guard<std::mutex> lock(callbacks_mutex_);
  callbacks_.erase(std::remove_if(callbacks_.begin(), callbacks_.end(),
                                  [&](const std::pair<NameList, std::weak_ptr<Callback>> callback_pair) {
                                    return callback_pair.second.expired();
                                  }),
                   callbacks_.end());

  // Call all callbacks with their appropriate configuration
  for (auto callback : callbacks_) {
    auto values = GetConfiguration(callback.first);
    if (auto callback_pointer = callback.second.lock()) {
      (*callback_pointer)(values);
    }
  }
}

Configuration::ValueList Configuration::GetConfiguration(NameList &required_config) {
  // Iterate through the XML file and grab the requested configurations from required_config
  // We shall assume that it takes more time to iterate through the XML file than it does to iterate through the string
  // vector, so we will have the XML iteration in the outer loop
  ValueList response_config;

  tinyxml2::XMLElement *root_element = config_document_.FirstChildElement();
  for (tinyxml2::XMLElement *element = root_element->FirstChildElement(); element != NULL;
       element = element->NextSiblingElement()) {
    for (const auto &item : required_config) {
      if (item.compare(element->Value()) == 0) {
        // Found the item, add it to the response vector and move on
        response_config.push_back(std::pair<std::string, std::string>(item, element->GetText()));
      }
    }
  }
  return response_config;
}

void Configuration::MonitorForChanges(unsigned int period_ms) {
  // Get the file modification date as a starting point
  file_monitor_running_ = true;
  auto last_modified_date = std::filesystem::last_write_time(config_filename_);
  while (file_monitor_running_) {
    std::this_thread::sleep_for(std::chrono::milliseconds(period_ms));

    auto modified_date = std::filesystem::last_write_time(config_filename_);
    if (last_modified_date != modified_date) {
      last_modified_date = modified_date;

      // File has been modified, dispatch new configuration to all listeners
      UpdateListeners();
    }
  }
}

void Configuration::DisableMonitor(void) { file_monitor_running_ = false; }

}  // namespace gramConfig
