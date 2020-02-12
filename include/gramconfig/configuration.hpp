#ifndef _CONFIG_CONFIGURATION_H_
#define _CONFIG_CONFIGURATION_H_

#include <tinyxml2.h>

#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace config {

class Configurable;

class Configuration {
 public:
  /** A list of configurations and values type
   * The first element in the pair is the name of the configuration, with the second element being it's string value
   */
  typedef std::vector<std::pair<std::string, std::string>> ValueList;

  /** A list of configuration names type
   */
  typedef std::vector<std::string> NameList;

  /** Configuration callback function type
   */
  typedef std::function<void(ValueList)> Callback;

  /** Constructor
   * \param xml_filename The name of the XML configuration file
   */
  Configuration(const std::string &xml_filename);

  /** Destructor
   */
  ~Configuration();

  /** Add a new configuration listener
   * This adds a number of configurations that need to be provided to the supplied callback function when the
   * configuration is updated
   * \param configurations NameList of configuration names
   * \param callback Callback to be called when the configuration changes
   * \return A shared pointer that can be used as a token. UpdateListeners ensures the token is still in existence
   * before calling the callback.
   */
  std::shared_ptr<void> AddListener(const NameList &configurations, Callback callback);

  /** Add a new configuration listener
   * This adds a number of configurations that need to be provided to the supplied callback function when the
   * configuration is updated
   * \param configurations NameList of configuration names
   * \param callback Reference to a class that implements Configurable
   * \return A shared pointer that can be used as a token. UpdateListeners ensures the token is still in existence
   * before calling the callback.
   */
  std::shared_ptr<void> AddListener(const NameList &configurations, Configurable &callback);

  /** Update all listeners with configuration
   */
  void UpdateListeners(void);

  /** Get a specific configuration
   * Request a specific list of configuration values without having to register any listeners
   * \param required_config NameList of configuration names
   * \return A ValueList of the requested configuration and their values
   */
  ValueList GetConfiguration(NameList &required_config);

  /** Continously monitor the configuration file for changes
   * Sits in a forever loop checking the configuration file for changes and notifying any registered listeners upon
   * change. This should be called in a separate thread to the main application
   * \param period_ms Number of milliseconds to wait between file polls
   */
  void MonitorForChanges(unsigned int period_ms);

 private:
  /** List of callbacks containing a weak pointer to the callback function and the relevant NameList of configurations
   */
  std::vector<std::pair<NameList, std::weak_ptr<Callback>>> callbacks_;

  /** Configuration XML filename
   */
  const std::string config_filename_;

  /** TinyXML document
   */
  tinyxml2::XMLDocument config_document_;

  /** File monitor running flag
   * This flag must be true to keep the file monitor runnign
   */
  bool file_monitor_running_;

  /** Mutex for locking access to the callbacks_ vector in multithreaded applications
   */
  std::mutex callbacks_mutex_;
};

}  // namespace config

#endif  // _CONFIG_CONFIGURATION_H_