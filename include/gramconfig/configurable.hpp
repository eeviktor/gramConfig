#ifndef _CONFIG_CONFIGURABLE_H_
#define _CONFIG_CONFIGURABLE_H_

#include <memory>

#include "gramconfig/configuration.hpp"

namespace gramConfig {

class Configurable {
 public:
  /** Constructor
   * \param configuration Reference to the configuration manager that is providing the configuration
   */
  Configurable(std::shared_ptr<Configuration> configuration) : configuration_(configuration) {}

  /** Register a number of configurations that this class needs
   * \param configurations List of configuration names
   */
  void ConfigurationRegister(const Configuration::NameList &configurations) {
    configuration_token_ = configuration_->AddListener(configurations, *this);
  }

  /** Callback for configuration manager to call and update the class
   * \param config List of configuration names and values
   */
  virtual void ConfigurationUpdate(Configuration::ValueList config) = 0;

 private:
  /** Reference to the configuration manager that is providing the configuration
   */
  std::shared_ptr<Configuration> configuration_;

  /** Token that ensures that when this object goes out of scope, the configuration class removes it's registration
   * automatically and safely
   */
  std::shared_ptr<void> configuration_token_;
};

}  // namespace gramConfig

#endif  // _CONFIG_CONFIGURABLE_H_