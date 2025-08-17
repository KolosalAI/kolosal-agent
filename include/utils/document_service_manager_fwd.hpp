#pragma once

// Forward declaration approach to avoid including heavy dependencies
namespace kolosal {
    struct DatabaseConfig;
}

namespace kolosal {
namespace retrieval {
    class DocumentService;
}
}

namespace kolosal {
namespace agents {

/**
 * @brief Manager for DocumentService instances in the agent context
 */
class DocumentServiceManager {
public:
    static DocumentServiceManager& get_Instance();
    
    /**
     * @brief Initialize the document service with configuration
     * @param config Database configuration for the service
     * @return true if initialization was successful
     */
    bool initialize(const kolosal::DatabaseConfig& config);
    
    /**
     * @brief Get the document service instance
     * @return Reference to the document service
     * @throws std::runtime_error if service is not available
     */
    kolosal::retrieval::DocumentService& getDocument_Service();
    
    /**
     * @brief Check if the document service is available and ready
     * @return true if service is available
     */
    bool is_Available() const;

private:
    DocumentServiceManager() = default;
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace agents
} // namespace kolosal
