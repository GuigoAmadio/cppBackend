#pragma once

#include <string>
#include <map>

namespace Domain {
namespace Workspace {

/**
 * @brief Entity: WorkspaceSettings
 * Configurações de um workspace (armazena como JSON string)
 */
class WorkspaceSettings {
public:
    WorkspaceSettings() : settings_("{}") {}
    explicit WorkspaceSettings(const std::string& jsonSettings) 
        : settings_(jsonSettings) {}

    const std::string& getSettings() const { return settings_; }
    void setSettings(const std::string& settings) { settings_ = settings; }

    // Helper methods para configurações comuns
    void setDefaultLanguage(const std::string& language) {
        // Implementar parsing/modification de JSON quando necessário
    }

    void setTimezone(const std::string& timezone) {
        // Implementar parsing/modification de JSON quando necessário
    }

private:
    std::string settings_; // JSON string
};

} // namespace Workspace
} // namespace Domain

