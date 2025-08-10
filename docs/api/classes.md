# Classes API Reference

Generated on 2025-08-08 14:06:25

## Agent

**File:** `naming_backup\include\agent\agent_data.hpp`  
**Line:** 155  

**Description:** /** * @brief Represents agent functionality */

### Methods

#### get__agent_id

```cpp
string get__agent_id() const
```

#### get__agent_name

```cpp
string get__agent_name() const
```

#### get__agent_type

```cpp
string get__agent_type() const
```

#### is__running

```cpp
bool is__running() const
```

#### set__running

```cpp
void set__running(bool running)
```

#### get__capabilities

```cpp
vector<std::string> get__capabilities() const
```

---

## AgentData

**File:** `naming_backup\include\agent\agent_data.hpp`  
**Line:** 57  

**Description:** /** * @brief Represents agent data functionality */

### Methods

#### set

```cpp
void set(const std::string& key, const std::string& value)
```

/** * @brief Set * @return void Description of return value */

#### set

```cpp
void set(const std::string& key, int value)
```

/** * @brief Set * @return void Description of return value */

#### set

```cpp
void set(const std::string& key, double value)
```

/** * @brief Set * @return void Description of return value */

#### set

```cpp
void set(const std::string& key, bool value)
```

/** * @brief Set * @return void Description of return value */

#### set

```cpp
void set(const std::string& key, const std::vector<std::string>& value)
```

/** * @brief Set * @return void Description of return value */

#### set

```cpp
void set(const std::string& key, const AgentData& value)
```

/** * @brief Set * @return void Description of return value */

#### set

```cpp
void set(const std::string& key, const AgentDataValue& value)
```

/** * @brief Set * @return void Description of return value */

#### get__string

```cpp
string get__string(const std::string& key, const std::string& default_val = "") const
```

Get methods with defaults

#### get__int

```cpp
int get__int(const std::string& key, int default_val = 0) const
```

Get methods with defaults

#### get__double

```cpp
double get__double(const std::string& key, double default_val = 0.0) const
```

Get methods with defaults

#### get__bool

```cpp
bool get__bool(const std::string& key, const bool default_val = false) const
```

Get methods with defaults

#### get__array_string

```cpp
vector<std::string> get__array_string(const std::string& key) const
```

#### has__key

```cpp
bool has__key(const std::string& key) const
```

Utility methods

#### clear

```cpp
void clear()
```

Utility methods

#### get__all_keys

```cpp
vector<std::string> get__all_keys() const
```

Utility methods

#### get__keys

```cpp
vector<std::string> get__keys() const
```

Utility methods

#### get__all_keys

```cpp
return get__all_keys()
```

Utility methods

#### to_string

```cpp
string to_string() const
```

#### to_json

```cpp
json to_json() const
```

JSON conversion

#### from_json

```cpp
void from_json(const nlohmann::json& json_data)
```

JSON conversion

---

## ChunkData

**File:** `naming_backup\kolosal-server\include\kolosal\models\chunking_response_model.hpp`  
**Line:** 26  

**Description:** /** * @brief Model for a single chunk in the response */

### Methods

#### to_json

```cpp
json to_json() const
```

/** * @brief Converts the chunk to JSON * @return JSON representation */

---

## ChunkingRequest

**File:** `naming_backup\kolosal-server\include\kolosal\models\chunking_request_model.hpp`  
**Line:** 27  

**Description:** /** * @brief Model for chunking request * * This model represents the request body for the /chunking endpoint */

---

## ChunkingResponse

**File:** `naming_backup\kolosal-server\include\kolosal\models\chunking_response_model.hpp`  
**Line:** 61  

**Description:** /** * @brief Model for chunking response * * This model represents the response body for the /chunking endpoint */

### Methods

#### add_Chunk

```cpp
void add_Chunk(const ChunkData& chunk)
```

/** * @brief Adds a chunk to the response * @param chunk The chunk data to add */

#### set_Usage

```cpp
void set_Usage(int original_tokens, int total_chunk_tokens, float processing_time_ms)
```

* @param original_tokens Number of tokens in original text * @param total_chunk_tokens Total tokens across all chunks * @param processing_time_ms Processing time in milliseconds */

---

## ChunkingService

**File:** `naming_backup\kolosal-server\include\kolosal\retrieval\chunking_types.hpp`  
**Line:** 39  

**Description:** /** * @brief Service for text chunking operations * * This service provides both regular and semantic chunking capabilities * for text documents. It supports concurrent processing and is thread-safe. */

### Methods

#### generateBase_Chunks

```cpp
vector<std::string> generateBase_Chunks(
        const std::string& text,
        const std::vector<std::string>& tokens,
        int chunk_size,
        int overlap
    ) const
```

* @param chunk_size Number of tokens per chunk * @param overlap Number of tokens to overlap between chunks * @return Vector of text chunks */

#### cosine_Similarity

```cpp
float cosine_Similarity(const std::vector<float>& a, const std::vector<float>& b)
```

* @param a First embedding vector * @param b Second embedding vector * @return Cosine similarity value */

#### estimateToken_Count

```cpp
int estimateToken_Count(const std::string& text)
```

* * @param text Input text * @return Estimated token count */

#### reconstruct_Text

```cpp
string reconstruct_Text(const std::vector<std::string>& tokens)
```

* * @param tokens Vector of tokens * @return Reconstructed text */

#### validateChunking_Parameters

```cpp
void validateChunking_Parameters(int chunk_size, int overlap, int max_tokens, float similarity_threshold) const
```

Internal helper methods

#### extractToken_Subset

```cpp
vector<std::string> extractToken_Subset(const std::vector<std::string>& tokens, int start, int end) const
```

Internal helper methods

---

## CollaborationPattern

**File:** `naming_backup\include\agent\agent_orchestrator.hpp`  
**Line:** 73  

**Description:** Agent collaboration pattern

---

## ComponentLogger

**File:** `naming_backup\include\logging_utilities.hpp`  
**Line:** 44  

**Description:** /** * @brief Component-based logger wrapper for easier usage */

### Methods

#### trace

```cpp
void trace(const std::string& format, arguments&&... arguments)
```

/** * @brief Perform trace operation * @return void Description of return value */

#### debug

```cpp
void debug(const std::string& format, arguments&&... arguments)
```

/** * @brief Perform debug operation * @return void Description of return value */

#### informationrmationrmationrmationrmationrmation

```cpp
void informationrmationrmationrmationrmationrmation(const std::string& format, arguments&&... arguments)
```

/** * @brief Perform informationrmationrmationrmationrmationrmation operation * @return void Description of return value */

#### warn

```cpp
void warn(const std::string& format, arguments&&... arguments)
```

/** * @brief Perform warn operation * @return void Description of return value */

#### error

```cpp
void error(const std::string& format, arguments&&... arguments)
```

/** * @brief Perform error operation * @return void Description of return value */

#### fatal

```cpp
void fatal(const std::string& format, arguments&&... arguments)
```

/** * @brief Perform fatal operation * @return void Description of return value */

#### trace

```cpp
void trace(const std::string& message)
```

/** * @brief Perform trace operation * @return void Description of return value */

#### debug

```cpp
void debug(const std::string& message)
```

/** * @brief Perform debug operation * @return void Description of return value */

#### info

```cpp
void info(const std::string& message)
```

/** * @brief Perform info operation * @return void Description of return value */

#### warn

```cpp
void warn(const std::string& message)
```

/** * @brief Perform warn operation * @return void Description of return value */

#### error

```cpp
void error(const std::string& message)
```

/** * @brief Perform error operation * @return void Description of return value */

#### fatal

```cpp
void fatal(const std::string& message)
```

/** * @brief Perform fatal operation * @return void Description of return value */

---

## ConsoleAppender

**File:** `naming_backup\include\logger_system.hpp`  
**Line:** 112  

**Description:** /** * @brief Console appender for logging to stdout/stderr */

### Methods

#### getColor_Code

```cpp
string getColor_Code(LogLevel level) const
```

#### getReset_Code

```cpp
string getReset_Code() const
```

---

## DOCXParser

**File:** `naming_backup\kolosal-server\include\kolosal\retrieval\parse_docx.hpp`  
**Line:** 34  

**Description:** /** * @brief Represents d o c x parser functionality */

### Methods

#### parse_docx

```cpp
string parse_docx(const std::string &file_path)
```

Synchronous parsing

#### parse_docx_from_bytes

```cpp
string parse_docx_from_bytes(const unsigned char *data, size_t size)
```

Parse from memory buffer

#### parse_docx_async

```cpp
future<std::string> parse_docx_async(const std::string &file_path)
```

Asynchronous parsing

#### is__valid_docx

```cpp
bool is__valid_docx(const std::string &file_path)
```

Utility functions

#### get__page_count

```cpp
size_t get__page_count(const std::string &file_path)
```

Utility functions

#### parse_docx_internal

```cpp
string parse_docx_internal(const std::string &file_path)
```

Thread-safe parsing methods

#### parse_docx_from_bytes_internal

```cpp
string parse_docx_from_bytes_internal(const unsigned char *data, size_t size)
```

Memory-based parsing methods

#### file_exists

```cpp
bool file_exists(const std::string &file_path)
```

Utility functions

#### has__docx_extension

```cpp
bool has__docx_extension(const std::string &file_path)
```

Utility functions

---

## DefaultLogFormatter

**File:** `naming_backup\include\logger_system.hpp`  
**Line:** 81  

**Description:** /** * @brief Default log formatter with customizable formatting */

### Methods

#### levelTo_String

```cpp
string levelTo_String(LogLevel level) const
```

#### format_Timestamp

```cpp
string format_Timestamp(const std::chrono::system_clock::time_point& timestamp) const
```

---

## DocumentAgentService

**File:** `naming_backup\include\document_agent_service.hpp`  
**Line:** 49  

**Description:** /** * @brief Provides document agent services */

### Methods

#### from_json

```cpp
void from_json(const json& j)
```

#### validate

```cpp
bool validate() const
```

#### to_json

```cpp
json to_json() const
```

#### from_json

```cpp
void from_json(const json& j)
```

#### validate

```cpp
bool validate() const
```

#### to_json

```cpp
json to_json() const
```

#### from_json

```cpp
void from_json(const json& j)
```

#### validate

```cpp
bool validate() const
```

#### to_json

```cpp
json to_json() const
```

#### from_json

```cpp
void from_json(const json& j)
```

#### validate

```cpp
bool validate() const
```

#### to_json

```cpp
json to_json() const
```

#### processBulk_Documents

```cpp
future<BulkDocumentResponse> processBulk_Documents(const BulkDocumentRequest& request)
```

Core service methods

#### processBulk_Retrieval

```cpp
future<BulkRetrievalResponse> processBulk_Retrieval(const BulkRetrievalRequest& request)
```

Core service methods

#### search_Documents

```cpp
future<DocumentSearchResponse> search_Documents(const DocumentSearchRequest& request)
```

Core service methods

#### upload_Document

```cpp
future<DocumentUploadResponse> upload_Document(const DocumentUploadRequest& request)
```

Core service methods

#### list_Collections

```cpp
future<json> list_Collections()
```

Collection management

#### delete_Collection

```cpp
future<json> delete_Collection(const std::string& name)
```

Collection management

#### getCollection_Info

```cpp
future<json> getCollection_Info(const std::string& name)
```

Collection management

#### generateError_Message

```cpp
string generateError_Message(const std::string& operation, const std::exception& e)
```

---

## DocumentParser

**File:** `naming_backup\kolosal-server\include\kolosal\retrieval\parse_pdf.hpp`  
**Line:** 56  

**Description:** /** * @brief Represents document parser functionality */

### Methods

#### parse_pdf

```cpp
ParseResult parse_pdf(const std::string &file_path,
                                     const PDFParseMethod method = PDFParseMethod::Fast,
                                     const std::string &language = "eng",
                                     ProgressCallback progress_cb = nullptr)
```

Synchronous parsing

#### parse_pdf_from_bytes

```cpp
ParseResult parse_pdf_from_bytes(const unsigned char *data, size_t size,
                                                const PDFParseMethod method = PDFParseMethod::Fast,
                                                const std::string &language = "eng",
                                                ProgressCallback progress_cb = nullptr)
```

Parse from memory buffer

#### parse_pdf_async

```cpp
future<ParseResult> parse_pdf_async(const std::string &file_path,
                                                        const PDFParseMethod method = PDFParseMethod::Fast,
                                                        const std::string &language = "eng",
                                                        ProgressCallback progress_cb = nullptr)
```

Asynchronous parsing

#### is__valid_pdf

```cpp
bool is__valid_pdf(const std::string &file_path)
```

Utility functions

#### get__page_count

```cpp
size_t get__page_count(const std::string &file_path)
```

Utility functions

#### parse_pdf_fast

```cpp
ParseResult parse_pdf_fast(const std::string &file_path,
                                          const ProgressCallback progress_cb = nullptr)
```

Thread-safe parsing methods

#### parse_pdf_ocr

```cpp
ParseResult parse_pdf_ocr(const std::string &file_path,
                                         const std::string &language,
                                         const ProgressCallback progress_cb = nullptr)
```

Thread-safe parsing methods

#### parse_pdf_visual

```cpp
ParseResult parse_pdf_visual(const std::string &file_path,
                                            const ProgressCallback progress_cb = nullptr)
```

#### parse_pdf_fast_from_bytes

```cpp
ParseResult parse_pdf_fast_from_bytes(const unsigned char *data, size_t size,
                                                     const ProgressCallback progress_cb = nullptr)
```

Memory-based parsing methods

#### parse_pdf_ocr_from_bytes

```cpp
ParseResult parse_pdf_ocr_from_bytes(const unsigned char *data, size_t size,
                                                    const std::string &language,
                                                    const ProgressCallback progress_cb = nullptr)
```

Memory-based parsing methods

#### parse_pdf_visual_from_bytes

```cpp
ParseResult parse_pdf_visual_from_bytes(const unsigned char *data, size_t size,
                                                       const ProgressCallback progress_cb = nullptr)
```

#### extract_text_from_page

```cpp
string extract_text_from_page(const PoDoFo::PdfMemDocument& doc, int page_num)
```

Utility functions

#### file_exists

```cpp
bool file_exists(const std::string &file_path)
```

Utility functions

#### has__pdf_extension

```cpp
bool has__pdf_extension(const std::string &file_path)
```

Utility functions

---

## DocumentServiceManager

**File:** `naming_backup\include\document_service_manager.hpp`  
**Line:** 28  

**Description:** /** * @brief Manager for DocumentService instances in the agent context */

### Methods

#### initialize_ialize

```cpp
bool initialize_ialize(const DatabaseConfig& config)
```

/** * @brief Initialize the document service with configuration */

#### is_Available

```cpp
bool is_Available() const
```

/** * @brief Check if document service is available */

---

## DocumentType

**File:** `naming_backup\kolosal-server\include\kolosal\routes\retrieval\parse_document_route.hpp`  
**Line:** 33  

**Description:** * @brief Represents parse document route functionality */

---

## DownloadsRoute

**File:** `naming_backup\kolosal-server\include\kolosal\routes\downloads_route.hpp`  
**Line:** 23  

**Description:** /** * @brief Represents downloads route functionality */

### Methods

#### extractModel_Id

```cpp
string extractModel_Id(const std::string& path)
```

Extract model ID from path like /downloads/{model_id}

#### request

```cpp
progress request (GET)
        void handleSingle_Download(SocketType sock, const std::string& model_id)
```

Handle single download

#### request

```cpp
status request (GET)
        void handleAll_Downloads(SocketType sock)
```

Handle all downloads

#### download

```cpp
single download (DELETE/POST)
        void handleCancel_Download(SocketType sock, const std::string& model_id)
```

Handle cancel

#### downloads

```cpp
all downloads (DELETE/POST)
        void handleCancelAll_Downloads(SocketType sock)
```

Handle cancel

#### download

```cpp
pause download (POST)
        void handlePause_Download(SocketType sock, const std::string& model_id)
```

Handle

#### download

```cpp
resume download (POST)
        void handleResume_Download(SocketType sock, const std::string& model_id)
```

Handle

---

## EmbeddingErrorResponse

**File:** `naming_backup\kolosal-server\include\kolosal\models\embedding_response_model.hpp`  
**Line:** 123  

**Description:** /** * @brief Error response for embedding requests */

### Methods

#### to_json

```cpp
json to_json() const
```

#### from_json

```cpp
void from_json(const nlohmann::json& j)
```

---

## EmbeddingInferenceService

**File:** `naming_backup\kolosal-server\inference\src\inference.cpp`  
**Line:** 1527  

**Description:** EmbeddingInferenceService (Optimized for Embedding Models)

### Methods

#### lock

```cpp
cout << "[INFERENCE] [EMBEDDING] Initializing EmbeddingInferenceService with batch size: " << g_params.n_batch << std::endl
			std::cout << "[INFERENCE] [EMBEDDING] Context n_ubatch: " << llama_n_ubatch(context) << std::endl
#endif

			// Always enable embeddings for this service
			llama_set_embeddings(context, true)
			// Set non-causal attention for embedding models
			llama_set_causal_attn(context, false)

			batch = llama_batch_init(params.n_ctx, 0, params.n_parallel)

			std::thread inferenceThread(&EmbeddingInferenceService::start, this)
			inferenceThread.detach()
		}

		~EmbeddingInferenceService()
		
			try 
				stop()

				// Wait for any remaining jobs to complete with timeout
				auto timeout = std::chrono::steady_clock::now() + std::chrono::seconds(5) // Reduced timeout
				while (std::chrono::steady_clock::now() < timeout)
				
					
						std::lock_guard<std::mutex> lock(mtx)
```

#### lock

```cpp
unique_lock<std::mutex> lock(mtx)
```

#### processBatchEmbeddings

```cpp
efficiency
				processBatchEmbeddings(current_jobs)
```

Process embedding jobs in batches for

#### jobLock

```cpp
lock_guard<std::mutex> jobLock(job->mtx)
```

Not supported for embedding service

#### submitJob

```cpp
service
			submitJob(params, job)
```

Not supported for embedding

#### jobLock

```cpp
cout << "[INFERENCE] [EMBEDDING] Submitting embedding job" << std::endl
#endif

			if (!params.isValid())
			
				std::lock_guard<std::mutex> jobLock(job->mtx)
```

#### lock

```cpp
lock_guard<std::mutex> lock(mtx)
```

#### processBatchEmbeddings

```cpp
void processBatchEmbeddings(std::vector<std::shared_ptr<Job>> &current_jobs)
```

#### for

```cpp
jobs
			for (auto &job : current_jobs)
```

Filter embedding

#### jobLock

```cpp
lock_guard<std::mutex> jobLock(job->mtx)
```

Filter embedding jobs

#### processBatch

```cpp
efficiency
				processBatch(embedding_jobs)
```

Process embeddings in batch for

#### for

```cpp
failed
				for (auto &job : embedding_jobs)
```

Mark all jobs as

#### jobLock

```cpp
lock_guard<std::mutex> jobLock(job->mtx)
```

Mark all jobs as failed

#### lock

```cpp
lock_guard<std::mutex> lock(mtx)
```

Remove completed jobs

#### jobLock

```cpp
lock_guard<std::mutex> jobLock(job->mtx)
```

#### processBatch

```cpp
void processBatch(std::vector<std::shared_ptr<Job>> &embedding_jobs)
```

#### common_batch_clear

```cpp
cache
			common_batch_clear(batch)
```

Clear batch and KV

#### llama_set_embeddings

```cpp
generation
			llama_set_embeddings(context, true)
```

Set up context for embedding

#### jobLock

```cpp
i < embedding_jobs.size() ++i)
			
				auto &job = embedding_jobs[i]

				try
				
					std::string input
					bool normalize = true
					
						std::lock_guard<std::mutex> jobLock(job->mtx)
```

Tokenize all inputs

#### jobLock

```cpp
lock_guard<std::mutex> jobLock(job->mtx)
```

#### jobLock

```cpp
cout << "[INFERENCE] [EMBEDDING] Truncated input to " << max_tokens_per_sequence << " tokens" << std::endl
#endif
					}

					all_tokens.push_back(tokens)
					job_indices.push_back(i)
				}
				catch (const std::exception &e)
				
					std::lock_guard<std::mutex> jobLock(job->mtx)
```

#### if

```cpp
n_ubatch
					if (batch.n_tokens >= static_cast<int>(context_n_ubatch))
```

For non-causal attention, ensure we don't exceed

#### if

```cpp
size
			if (batch.n_tokens == 0)
```

Ensure we have tokens to process and validate batch

#### if

```cpp
attention
			if (batch.n_tokens > static_cast<int>(context_n_ubatch))
```

Final check: ensure batch size doesn't exceed n_ubatch for non-causal

#### if

```cpp
batch
			if (llama_decode(context, batch) != 0)
```

Decode the

#### jobLock

```cpp
seq < job_indices.size() ++seq)
			
				auto &job = embedding_jobs[job_indices[seq]]

				try
				
					bool normalize
					
						std::lock_guard<std::mutex> jobLock(job->mtx)
```

Extract embeddings for each sequence

#### jobLock

```cpp
lock_guard<std::mutex> jobLock(job->mtx)
```

#### embedding

```cpp
vector<float> embedding(embd, embd + n_embd)
```

Copy and normalize embedding

#### jobLock

```cpp
lock_guard<std::mutex> jobLock(job->mtx)
```

#### jobLock

```cpp
cout << "[INFERENCE] [EMBEDDING] Generated " << n_embd << "-dimensional embedding for sequence " << seq << std::endl
#endif
				}
				catch (const std::exception &e)
				
					std::lock_guard<std::mutex> jobLock(job->mtx)
```

#### llama_set_embeddings

```cpp
embeddings
			llama_set_embeddings(context, true)
```

Restore context settings after processing

---

## EmbeddingRequest

**File:** `naming_backup\kolosal-server\include\kolosal\models\embedding_request_model.hpp`  
**Line:** 30  

**Description:** /** * @brief Model for embedding request * * This model represents the request body for the /v1/embeddings endpoint * following the OpenAI embeddings API specification. */

### Methods

#### getInput_Texts

```cpp
vector<std::string> getInput_Texts() const
```

/** * @brief Gets the input as a vector of strings * @return Vector of input strings */

#### hasMultiple_Inputs

```cpp
bool hasMultiple_Inputs() const
```

/** * @brief Checks if the request has multiple inputs * @return true if multiple inputs, false if single input */

---

## EmbeddingResponse

**File:** `naming_backup\kolosal-server\include\kolosal\models\embedding_response_model.hpp`  
**Line:** 70  

**Description:** /** * @brief Model for embedding response * * This model represents the response for the /v1/embeddings endpoint * following the OpenAI embeddings API specification. */

### Methods

#### add_Embedding

```cpp
void add_Embedding(const std::vector<float>& embedding, int index)
```

* @brief Add an embedding result to the response * @param embedding Vector of floats representing the embedding * @param index Index of this embedding in the batch */

#### set_Usage

```cpp
void set_Usage(int prompt_tokens)
```

/** * @brief Sets the usage statistics * @param prompt_tokens Number of tokens in the input */

---

## EnginesRoute

**File:** `naming_backup\kolosal-server\include\kolosal\routes\engines_route.hpp`  
**Line:** 35  

**Description:** * * Supported endpoints: * - GET /engines or /v1/engines - List available engines with default engine information * - POST /engines or /v1/engines - Add new engine * - PUT /engines or /v1/engines - Set default engine */ /** * @brief Represents engines route functionality */

### Methods

#### handleGet_Engines

```cpp
void handleGet_Engines(SocketType sock)
```

/** * @brief Handle GET request to list available inference engines * @param sock Socket to send response to */

#### handleAdd_Engine

```cpp
void handleAdd_Engine(SocketType sock, const std::string &body)
```

* @brief Handle POST request to add a new inference engine * @param sock Socket to send response to * @param body JSON request body containing engine configuration */

#### handleSetDefault_Engine

```cpp
void handleSetDefault_Engine(SocketType sock, const std::string &body)
```

* @brief Handle PUT request to set the default inference engine * @param sock Socket to send response to * @param body JSON request body containing default engine name */

---

## FileAppender

**File:** `naming_backup\include\logger_system.hpp`  
**Line:** 130  

**Description:** /** * @brief File appender for logging to files with rotation support */

### Methods

#### is_Open

```cpp
bool is_Open() const
```

#### rotate_File

```cpp
void rotate_File()
```

#### open_File

```cpp
void open_File()
```

---

## HealthStatusRoute

**File:** `naming_backup\kolosal-server\include\kolosal\routes\health_status_route.hpp`  
**Line:** 22  

**Description:** /** * @brief Represents health status route functionality */

---

## HttpClient

**File:** `naming_backup\include\routes\http_client.hpp`  
**Line:** 24  

**Description:** /** * @brief Simple HTTP client stub for making requests */

### Methods

#### makeStreaming_Request

```cpp
bool makeStreaming_Request(
        const std::string& url, 
        const std::string& body, 
        const std::map<std::string, std::string>& headers,
        std::function<bool(const std::string&)> callback)
```

#### get

```cpp
bool get(const std::string& url, std::string& response)
```

#### get

```cpp
bool get(const std::string& url, std::string& response, const std::vector<std::string>& headers)
```

#### post

```cpp
bool post(const std::string& url, const std::string& body, std::string& response)
```

#### post

```cpp
bool post(const std::string& url, const std::string& body, std::string& response, const std::vector<std::string>& headers)
```

#### delete_Request

```cpp
bool delete_Request(const std::string& url, const std::vector<std::string>& headers)
```

#### put

```cpp
bool put(const std::string& url, const std::string& body, std::string& response, const std::vector<std::string>& headers)
```

---

## InferenceLoader

**File:** `naming_backup\kolosal-server\include\kolosal\inference_loader.hpp`  
**Line:** 96  

**Description:** * at runtime. It provides functionality to: * - Configure available inference engines from configurationuration * - Load/unload engines dynamically * - Create engine instances * - Manage engine lifecycle */ /** * @brief Represents inference loader functionality */

### Methods

#### discovery

```cpp
plugin discovery (deprecated, not used)
     */
    explicit Inference_Loader(const std::string& plugins_dir = "")
```

/** * @brief Constructor * @param plugins_dir Directory for legacy

#### configure_Engines

```cpp
bool configure_Engines(const std::vector<InferenceEngineConfig>& engines)
```

* @brief Configure available inference engines from config * @param engines Vector of inference engine configurations * @return True if engines were configured successfully */

#### getAvailable_Engines

```cpp
vector<InferenceEngineInfo> getAvailable_Engines() const
```

/** * @brief Get list of available inference engines * @return Vector of engine informationrmation structures */

#### load

```cpp
to load (e.g., "cpu", "cuda")
     * @return True if the engine was loaded successfully
     */
    bool load_Engine(const std::string& engine_name)
```

/** * @brief Load a specific inference engine * @param engine_name Name of the engine

#### unload_Engine

```cpp
bool unload_Engine(const std::string& engine_name)
```

* @brief Unload a specific inference engine * @param engine_name Name of the engine to unload * @return True if the engine was unloaded successfully */

#### isEngine_Loaded

```cpp
bool isEngine_Loaded(const std::string& engine_name) const
```

* @brief Check if an engine is currently loaded * @param engine_name Name of the engine to check * @return True if the engine is loaded */

#### getLast_Error

```cpp
string getLast_Error() const
```

/** * @brief Get the last error message * @return Error message string */

#### directory

```cpp
plugins directory (DEPRECATED)
     * @deprecated This method is no longer used. Use configure_Engines() instead.
     * @param plugins_dir New plugins directory path
     */
    [[deprecated("Use configure_Engines() instead")]]
    void setPlugins_Directory(const std::string& plugins_dir)
```

/** * @brief Set the

#### directory

```cpp
plugins directory (DEPRECATED)
     * @deprecated This method is no longer used. Use configure_Engines() instead.
     * @return Current plugins directory path
     */
    [[deprecated("Use configure_Engines() instead")]]
    std::string getPlugins_Directory() const
```

/** * @brief Get the current

#### load_Library

```cpp
bool load_Library(const std::string& library_path, const std::string& engine_name)
```

* @param library_path Path to the shared library * @param engine_name Name of the engine * @return True if the library was loaded successfully */

#### unload_Library

```cpp
void unload_Library(const std::string& engine_name)
```

/** * @brief Unload a library * @param engine_name Name of the engine to unload */

#### setLast_Error

```cpp
void setLast_Error(const std::string& error) const
```

/** * @brief Set the last error message * @param error Error message */

---

## InferenceService

**File:** `naming_backup\kolosal-server\inference\src\inference.cpp`  
**Line:** 303  

**Description:** InferenceService Interface (Internal Use Only)

---

## InternetSearchRoute

**File:** `naming_backup\kolosal-server\include\kolosal\routes\retrieval\internet_search_route.hpp`  
**Line:** 59  

**Description:** /** * @brief Represents internet search route functionality */

### Methods

#### startWorker_Threads

```cpp
void startWorker_Threads()
```

#### stopWorker_Threads

```cpp
void stopWorker_Threads()
```

#### worker_Loop

```cpp
void worker_Loop()
```

#### buildSearch_Url

```cpp
string buildSearch_Url(const SearchRequest& request)
```

#### parseRequest_Body

```cpp
SearchRequest parseRequest_Body(const std::string& body)
```

#### validate_Request

```cpp
string validate_Request(const SearchRequest& request)
```

#### Write_Callback

```cpp
size_t Write_Callback(void* contents, size_t size, size_t nmemb, std::string* userp)
```

Static callback for libcurl

#### InternetSearch_Route

```cpp
explicit InternetSearch_Route(const SearchConfig& config)
```

Static callback for libcurl

---

## JobStatus

**File:** `naming_backup\include\task_job_manager.hpp`  
**Line:** 30  

---

## KolosalServerClient

**File:** `naming_backup\include\server_client_interface.h`  
**Line:** 24  

**Description:** /** * @brief Client for communicating with Kolosal Server */

### Methods

#### server

```cpp
Kolosal server (default: http://localhost:8080)
     * @param apiKey Optional API key for authentication
     */
    KolosalServer_Client(const std::string& baseUrl = "http://localhost:8080", const std::string& apiKey = "")
```

/** * @brief Constructor * @param baseUrl Base URL of the

#### on

```cpp
server on (default: 8080)
     * @return True if server started successfully or is already running, false otherwise
     */
    bool start_Server(const std::string& serverPath = "", const int port = 8080)
```

* @brief Start the Kolosal server in the background if not already running * @param serverPath Path to kolosal-server executable * @param port Port to run the

#### shutdown_Server

```cpp
bool shutdown_Server()
```

/** * @brief Gracefully shutdown the server via API call * @return True if shutdown request was sent successfully, false otherwise */

#### isServer_Healthy

```cpp
bool isServer_Healthy()
```

/** * @brief Check if the server is running and healthy * @return True if server is healthy, false otherwise */

#### seconds

```cpp
in seconds (default: 30)
     * @return True if server became healthy within timeout, false otherwise
     */
    bool waitForServer_Ready(const int timeoutSeconds = 30)
```

/** * @brief Wait for the server to become healthy * @param timeoutSeconds Maximum time to wait

#### add_Engine

```cpp
bool add_Engine(const std::string& engineId, const std::string& modelUrl, 
                   const std::string& modelPath)
```

* @param modelUrl URL to download the model from * @param modelPath Local path where the model will be saved * @return True if engine creation was initiated successfully, false otherwise */

#### get_Engines

```cpp
bool get_Engines(std::vector<std::string>& engines)
```

* @brief Get list of existing engines from the server * @param engines Output: vector of engine IDs that exist on the server * @return True if engines list was retrieved successfully, false otherwise */

#### getInference_Engines

```cpp
bool getInference_Engines(std::vector<std::tuple<std::string, std::string, std::string, std::string, bool>>& engines)
```

* @brief Get list of available inference engines from the server * @param engines Output: vector of inference engine informationrmation structures * @return True if inference engines list was retrieved successfully, false otherwise */

#### startup

```cpp
server startup (default: true)
     * @return True if engine was added successfully, false otherwise
     */
    bool addInference_Engine(const std::string& name, const std::string& libraryPath, const bool loadOnStartup = true)
```

/** * @brief Add an inference engine to the server * @param name Name of the inference engine * @param libraryPath Path to the engine library file * @param loadOnStartup Whether to load the engine on

#### getDefaultInference_Engine

```cpp
bool getDefaultInference_Engine(std::string& defaultEngine)
```

* @brief Get the current default inference engine from the server * @param defaultEngine Output: name of the default inference engine * @return True if default engine was retrieved successfully, false otherwise */

#### setDefaultInference_Engine

```cpp
bool setDefaultInference_Engine(const std::string& engineName)
```

* @brief Set the default inference engine on the server * @param engineName Name of the engine to set as default * @return True if default engine was set successfully, false otherwise */

#### engine_Exists

```cpp
bool engine_Exists(const std::string& engineId)
```

* @brief Check if an engine with the given ID already exists on the server * @param engineId Engine ID to check * @return True if engine exists, false otherwise */

#### percentage

```cpp
download percentage (0-100)
     * @param status Output: download status
     * @return True if progress was retrieved successfully, false otherwise
     */
    bool getDownload_Progress(const std::string& modelId, long long& downloadedBytes,
                           long long& totalBytes, double& percentage, std::string& status)
```

* @brief Get download progress for a specific model * @param modelId Model ID to check progress for * @param downloadedBytes Output: bytes downloaded so far * @param totalBytes Output: total bytes to download * @param percentage Output:

#### updates

```cpp
progress updates (percentage, status, downloadedBytes, totalBytes)
     * @param checkIntervalMs Interval between progress checks in milliseconds (default: 1000)
     * @return True if download completed successfully, false otherwise
     */
    bool monitorDownload_Progress(const std::string& modelId, 
                               std::function<void(double, const std::string&, long long, long long)> progressCallback,
                               const int checkIntervalMs = 1000)
```

/** * @brief Monitor download progress and provide updates * @param modelId Model ID to monitor * @param progressCallback Callback function called with

#### cancel_Download

```cpp
bool cancel_Download(const std::string& modelId)
```

* @brief Cancel a specific download * @param modelId Model ID of the download to cancel * @return True if cancellation request was successful, false otherwise */

#### pause_Download

```cpp
bool pause_Download(const std::string& modelId)
```

* @brief Pause a specific download * @param modelId Model ID of the download to pause * @return True if pause request was successful, false otherwise */

#### resume_Download

```cpp
bool resume_Download(const std::string& modelId)
```

* @brief Resume a specific download * @param modelId Model ID of the download to resume * @return True if resume request was successful, false otherwise */

#### cancelAll_Downloads

```cpp
bool cancelAll_Downloads()
```

/** * @brief Cancel all active downloads * @return True if cancellation request was successful, false otherwise */

#### info

```cpp
download info (modelId, status, progress, downloadedBytes, totalBytes)
     * @return True if downloads status was retrieved successfully, false otherwise
     */
    bool getAll_Downloads(std::vector<std::tuple<std::string, std::string, double, long long, long long>>& downloads)
```

/** * @brief Get status of all downloads * @param downloads Output: vector of

#### chat_Completion

```cpp
bool chat_Completion(const std::string& engineId, const std::string& message, std::string& response)
```

* @param message User message to send * @param response Output: assistant's response * @return True if chat completion was successful, false otherwise */

#### received

```cpp
chunk received (text, tps, ttft)
     * @return True if chat completion was successful, false otherwise
     */
    bool streamingChat_Completion(const std::string& engineId, const std::string& message, 
                               std::function<void(const std::string&, double, double)> responseCallback)
```

/** * @brief Send a streaming chat completion request to the server * @param engineId Engine ID to use for chat completion * @param message User message to send * @param responseCallback Callback function called for each token/

#### get_Logs

```cpp
bool get_Logs(std::vector<std::tuple<std::string, std::string, std::string>>& logs)
```

* @brief Get server logs * @param logs Output: vector of log entries with level, timestamp, and message * @return True if logs were retrieved successfully, false otherwise */

#### remove_Model

```cpp
bool remove_Model(const std::string& modelId)
```

* @brief Remove a model from the server * @param modelId Model ID to remove * @return True if model was removed successfully, false otherwise */

#### status

```cpp
model status (loaded, unloaded, etc.)
     * @param message Output: status message
     * @return True if status was retrieved successfully, false otherwise
     */
    bool getModel_Status(const std::string& modelId, std::string& status, std::string& message)
```

/** * @brief Get the status of a specific model * @param modelId Model ID to check * @param status Output:

#### endpoint

```cpp
API endpoint (e.g., "/v1/health")
     * @param response Output: response body
     * @return True if request was successful, false otherwise
     */
    bool makeGet_Request(const std::string& endpoint, std::string& response)
```

/** * @brief Make HTTP GET request to the server * @param endpoint

#### makePost_Request

```cpp
bool makePost_Request(const std::string& endpoint, const std::string& payload, std::string& response)
```

* @param payload JSON payload to send * @param response Output: response body * @return True if request was successful, false otherwise */

#### send

```cpp
to send (optional)
     * @param response Output: response body
     * @return True if request was successful, false otherwise
     */
    bool makeDelete_Request(const std::string& endpoint, const std::string& payload, std::string& response)
```

/** * @brief Make HTTP DELETE request to the server * @param endpoint API endpoint * @param payload JSON payload

#### makePut_Request

```cpp
bool makePut_Request(const std::string& endpoint, const std::string& payload, std::string& response)
```

* @param payload JSON payload to send * @param response Output: response body * @return True if request was successful, false otherwise */

#### parseJson_Value

```cpp
bool parseJson_Value(const std::string& jsonString, const std::string& key, std::string& value)
```

* @param key Key to extract from JSON * @param value Output: extracted value * @return True if parsing was successful, false otherwise */

#### parseJson_Number

```cpp
bool parseJson_Number(const std::string& jsonString, const std::string& key, double& value)
```

* @param key Key to extract from JSON * @param value Output: extracted number value * @return True if parsing was successful, false otherwise */

---

## LlamaInferenceService

**File:** `naming_backup\kolosal-server\inference\src\inference.cpp`  
**Line:** 315  

**Description:** LlamaInferenceService (CPU Implementation)

### Methods

#### lock

```cpp
cout << "Initializing batch with size of: " << g_params.n_batch << std::endl
#endif

			batch = llama_batch_init(params.n_ctx, 0, 1)

			inferenceThread = std::thread(&LlamaInferenceService::start, this)
		}

		~LlamaInferenceService()
		
			try 
				stop()

				// Wait for the inference thread to finish before cleaning up resources
				if (inferenceThread.joinable())
				
					// Use a timeout to prevent hanging
					auto future = std::async(std::launch::async, [this]() 
						inferenceThread.join()
					})
					
					if (future.wait_for(std::chrono::seconds(5)) == std::future_status::timeout) 
						// If thread doesn't join within timeout, detach it
						inferenceThread.detach()
					}
				}

				// Clean up all remaining jobs to prevent accessing freed resources
				
					std::lock_guard<std::mutex> lock(mtx)
```

#### jobLock

```cpp
lock_guard<std::mutex> jobLock(job->mtx)
```

#### lock

```cpp
unique_lock<std::mutex> lock(mtx)
```

#### jobLock

```cpp
lock_guard<std::mutex> jobLock(job->mtx)
```

Check for shutdown in the job processing loop

#### constraints

```cpp
grammar constraints (accept_grammar = false)
							common_sampler_accept(job->smpl, token, false)
```

Accept prompt tokens without

#### jobLock

```cpp
lock_guard<std::mutex> jobLock(job->mtx)
```

#### lock

```cpp
lock_guard<std::mutex> lock(mtx)
```

Ensure all remaining jobs are properly cleaned up when exiting

#### jobLock

```cpp
lock_guard<std::mutex> jobLock(job->mtx)
```

#### if

```cpp
endif

			if (!validateParameters(params, job))
```

#### jobLock

```cpp
lock_guard<std::mutex> jobLock(job->mtx)
```

#### lock

```cpp
lock_guard<std::mutex> lock(mtx)
```

#### lock

```cpp
cout << "[INFERENCE] Submitting job with sequence ID: " << params.seqId << std::endl
#endif

			submitJob(params, job)

			
				std::unique_lock<std::mutex> lock(job->mtx)
```

#### jobLock

```cpp
cout << "[INFERENCE] Submitting embedding job" << std::endl
#endif

			if (!params.isValid())
			
				std::lock_guard<std::mutex> jobLock(job->mtx)
```

#### jobLock

```cpp
lock_guard<std::mutex> jobLock(job->mtx)
```

Generate embedding for the input text

#### jobLock

```cpp
lock_guard<std::mutex> jobLock(job->mtx)
```

#### tc_params

```cpp
params tc_params(params.tools, params.toolChoice)
```

#### generateEmbedding

```cpp
vector<float> generateEmbedding(const std::string &input, bool normalize = true)
```

* @brief Generates embeddings for the given input text * @param input The input text to generate embeddings for * @return Vector of floats representing the embedding */

#### llama_set_embeddings

```cpp
mode
			llama_set_embeddings(context, true)
```

Enable embedding

#### llama_set_causal_attn

```cpp
directions
			llama_set_causal_attn(context, false)
```

For embedding models, we typically want to use non-causal attention This allows the model to attend to all tokens in both

#### limit

```cpp
token limit (embedding models usually have smaller context)
				const int max_tokens = std::min(n_ctx - 4, 8192)
```

Check for

#### if

```cpp
space
				if (static_cast<int>(tokens.size()) > max_tokens)
```

Check for token limit (embedding models usually have smaller context)

#### llama_kv_cache_clear

```cpp
generation
				llama_kv_cache_clear(context)
```

Clear the KV cache for clean embedding

#### embedding

```cpp
cout << "[INFERENCE] [EMBEDDING] Processing " << tokens.size() << " tokens" << std::endl
#endif

				// Decode the batch
				int ret = llama_decode(context, local_batch)
				if (ret != 0)
				
					llama_batch_free(local_batch)
					throw std::runtime_error("Failed to decode input for embedding generation")
				}

				// Get embedding dimension
				int n_embd = llama_model_n_embd(llama_get_model(context))
				if (n_embd <= 0)
				
					llama_batch_free(local_batch)
					throw std::runtime_error("Invalid embedding dimension")
				}

				// Get the embeddings using the appropriate method
				float *embd = nullptr
				const enum llama_pooling_type pooling_type = llama_pooling_type(context)

				if (pooling_type == LLAMA_POOLING_TYPE_NONE)
				
					// For token-level embeddings, get the last token's embedding
					embd = llama_get_embeddings_ith(context, local_batch.n_tokens - 1)
					if (!embd)
					
						llama_batch_free(local_batch)
						throw std::runtime_error("Failed to get token embeddings from model")
					}
				}
				else
				
					// For sequence-level embeddings (pooled), use sequence embeddings
					embd = llama_get_embeddings_seq(context, 0)
					if (!embd)
					
						// Fallback: try getting embeddings from the context directly
						embd = llama_get_embeddings(context)
						if (!embd)
						
							llama_batch_free(local_batch)
							throw std::runtime_error("Failed to get sequence embeddings from model")
						}
					}
				}

				// Copy embeddings to vector
				std::vector<float> embedding(embd, embd + n_embd)
```

#### requested

```cpp
if requested (common practice for embedding models)
				if (normalize)
```

Normalize embeddings

#### llama_batch_free

```cpp
up
				llama_batch_free(local_batch)
```

Clean

#### llama_set_embeddings

```cpp
attention
				llama_set_embeddings(context, false)
```

Disable embedding mode and restore causal

#### llama_set_embeddings

```cpp
error
				llama_set_embeddings(context, false)
```

Disable embedding mode and restore causal attention on

#### printLogits

```cpp
void printLogits(llama_context *ctx, size_t maxLogits = 10)
```

#### size

```cpp
vocabulary size (number of logits)
			const int n_vocab = llama_n_vocab(tokenizer->getVocab())
```

Get

#### text

```cpp
token text (you'll need to adapt this based on your tokenizer implementation)
				std::string token_text = tokenizer->decode(token_id)
```

Get the

#### if

```cpp
long
				if (escaped_text.length() > 31)
```

Truncate if too

#### validateParameters

```cpp
bool validateParameters(const CompletionParameters &params, std::shared_ptr<Job> job)
```

#### jobLock

```cpp
lock_guard<std::mutex> jobLock(job->mtx)
```

#### if

```cpp
grammar
			if (!params.jsonSchema.empty())
```

Handle JSON schema conversion to

#### jobLock

```cpp
lock_guard<std::mutex> jobLock(job->mtx)
```

Convert to grammar using llama.cpp's converter

#### directly

```cpp
provided directly (overrides JSON schema if both are provided)
			else if (!params.grammar.empty())
```

Set grammar if

#### jobLock

```cpp
lock_guard<std::mutex> jobLock(job->mtx)
```

#### loadSession

```cpp
bool loadSession(std::shared_ptr<Job> job)
```

#### getInputTokens

```cpp
bool getInputTokens(std::shared_ptr<Job> job)
```

#### ensureNonEmptyInput

```cpp
bool ensureNonEmptyInput(std::shared_ptr<Job> job)
```

#### checkCancellation

```cpp
bool checkCancellation(std::shared_ptr<Job> job)
```

#### ensureContextCapacity

```cpp
bool ensureContextCapacity(std::shared_ptr<Job> job)
```

#### sampleNextToken

```cpp
bool sampleNextToken(common_sampler *sampler, int &n_past, int &n_remain, std::vector<llama_token> &session_tokens, std::shared_ptr<Job> job, const std::string &path_session)
```

#### jobLock

```cpp
lock_guard<std::mutex> jobLock(job->mtx)
```

#### if

```cpp
token
				if (job->generatedTokens.empty())
```

Record first token timing if this is the first generated

#### sampleNextToken

```cpp
bool sampleNextToken(std::shared_ptr<Job> job)
```

#### if

```cpp
token
				if (job->generatedTokens.empty())
```

Record first token timing if this is the first generated

#### saveSession

```cpp
void saveSession(std::shared_ptr<Job> job)
```

#### load_kv_cache

```cpp
bool load_kv_cache(const std::string &path, std::vector<llama_token> &session_tokens, const int jobId)
```

#### if

```cpp
load
				if (!std::filesystem::exists(path))
```

Attempt to

#### printf

```cpp
cache
					printf("[KV] session file does not exist, will create.\n")
```

file doesn't exist => no old

#### if

```cpp
else if (std::filesystem::is_empty(path))
```

file doesn't exist => no old cache

#### printf

```cpp
new
					printf("[KV] session file is empty, new session.\n")
```

file is empty => treat as brand-

#### printf

```cpp
one
						printf("[KV] ERROR: Failed to load session file, deleting corrupt file and creating a new one.\n")
```

Loading failed - delete the corrupt file and create a new

#### printf

```cpp
DEBUG
					printf("[INFERENCE] [KV] loaded session with prompt size: %d tokens\n", (int)session_tokens.size())
```

The llama_state_load_file call gives us how many tokens were in that old session

#### matchSessionTokens

```cpp
size_t matchSessionTokens(std::shared_ptr<Job> job)
```

#### if

```cpp
DEBUG
				if (n_matching_session_tokens == job->embd_inp.size())
```

#### if

```cpp
else if (n_matching_session_tokens < (job->embd_inp.size() / 2))
```

#### prompt

```cpp
to prompt ("
							  << n_matching_session_tokens << " / " << job->embd_inp.size()
							  << ")
```

#### if

```cpp
endif

				if (job->session_tokens.size() > job->embd_inp.size() && n_matching_session_tokens > 0)
```

#### printf

```cpp
DEBUG
				printf("[INFERENCE] [KV] removed %d tokens from the cache\n", (int)(job->session_tokens.size() - n_matching_session_tokens))
```

#### llama_kv_self_seq_rm

```cpp
matched
				llama_kv_self_seq_rm(context, job->seqId, n_matching_session_tokens, -1 /*up to end*/)
```

Remove any "future" tokens that donât match i.e. we only keep the portion that

#### printf

```cpp
DEBUG
				printf("[INFERENCE] [KV] tokens decoded: %s\n", tokenizer->detokenize(job->session_tokens).c_str())
```

i.e. we only keep the portion that matched

#### kv_cache_seq_ltrim

```cpp
void kv_cache_seq_ltrim(llama_context *context, int n_keep, std::vector<llama_token> &session_tokens, int &n_past, const int id)
```

#### llama_kv_self_seq_rm

```cpp
endif

			llama_kv_self_seq_rm(context, id, n_keep, n_keep + n_discard)
```

---

## LoadingAnimation

**File:** `naming_backup\include\loading_animation_utils.hpp`  
**Line:** 21  

**Description:** /** * @brief Simple loading animation stub for console output */

### Methods

#### Loading_Animation

```cpp
explicit Loading_Animation(const std::string& message)
```

#### start

```cpp
void start()
```

#### stop

```cpp
void stop()
```

#### complete

```cpp
void complete(const std::string& message)
```

#### update_Message

```cpp
void update_Message(const std::string& message)
```

---

## LogAppender

**File:** `naming_backup\include\logger_system.hpp`  
**Line:** 102  

**Description:** /** * @brief Log appender interface for different output destinations */

---

## LogFormatter

**File:** `naming_backup\include\logger_system.hpp`  
**Line:** 72  

**Description:** /** * @brief Log formatter interface */

---

## LogLevel

**File:** `naming_backup\kolosal-server\include\kolosal\logger.hpp`  
**Line:** 26  

---

## Logger

**File:** `naming_backup\include\server_logger_integration.hpp`  
**Line:** 32  

**Description:** /** * @brief Logger interface for agent system (moved from removed logger.hpp) */

---

## LoggingConfig

**File:** `naming_backup\include\logging_utilities.hpp`  
**Line:** 159  

**Description:** /** * @brief Utility functions for setting up logging */

### Methods

#### setLog_Level

```cpp
void setLog_Level(const std::string& level)
```

/** * @brief Configure logging based on string level */

#### if

```cpp
else if (upper_level == "DEBUG")
```

#### if

```cpp
else if (upper_level == "information")
```

#### if

```cpp
else if (upper_level == "WARN" || upper_level == "WARNING")
```

#### if

```cpp
else if (upper_level == "ERROR")
```

#### if

```cpp
else if (upper_level == "FATAL")
```

#### if

```cpp
else if (upper_level == "OFF")
```

#### addFile_Logging

```cpp
bool addFile_Logging(const std::string& filename, 
                              const size_t max_file_size_mb = 10, 
                              size_t max_backup_files = 5)
```

/** * @brief Add file logging with optional rotation */

#### configureConsole_Logging

```cpp
void configureConsole_Logging(const bool use_colors = true, bool errors_to_stderr = true)
```

/** * @brief Configure console logging */

#### setupProduction_Logging

```cpp
void setupProduction_Logging(const std::string& log_file = "", const bool quiet_console = false)
```

/** * @brief Setup default logging configuration for production */

#### setupDevelopment_Logging

```cpp
void setupDevelopment_Logging(const std::string& log_file = "")
```

/** * @brief Setup logging for development */

#### if

```cpp
enabled
        
        if (!log_file.empty())
```

* @brief Setup logging for development */

#### logging

```cpp
minimal logging (errors only to stderr)
     */
    static void setupMinimal_Logging()
```

/** * @brief Setup

---

## LoggingScope

**File:** `naming_backup\include\logging_utilities.hpp`  
**Line:** 283  

**Description:** /** * @brief RAII-based logging scope */

---

## ModelType

**File:** `naming_backup\kolosal-server\include\kolosal\node_manager.h`  
**Line:** 229  

**Description:** /** * @brief Removes a model configuration from the config file * * @param engineId The engine ID to remove * @return True if config was updated successfully */

---

## PDFParseMethod

**File:** `naming_backup\kolosal-server\include\kolosal\retrieval\parse_pdf.hpp`  
**Line:** 34  

**Description:** /** * @brief Represents pdf mem document functionality */

---

## ParseDocumentRoute

**File:** `naming_backup\kolosal-server\include\kolosal\routes\retrieval\parse_document_route.hpp`  
**Line:** 26  

**Description:** /** * @brief Represents parse document route functionality */

### Methods

#### getDocument_Type

```cpp
DocumentType getDocument_Type(const std::string &path)
```

Thread-local storage for current path

#### getData_Key

```cpp
string getData_Key(DocumentType type)
```

#### getLog_Prefix

```cpp
string getLog_Prefix(DocumentType type)
```

#### sendJson_Response

```cpp
void sendJson_Response(SocketType sock, const nlohmann::json &response, const int status_code = 200)
```

#### parse_Request

```cpp
bool parse_Request(const std::string &body, nlohmann::json &request, SocketType sock)
```

#### validateDocument_Data

```cpp
bool validateDocument_Data(const nlohmann::json &request, const std::string &data_key, SocketType sock)
```

#### decode_Base64Data

```cpp
vector<unsigned char> decode_Base64Data(const std::string &base64_data, SocketType sock)
```

#### sendOptions_Response

```cpp
void sendOptions_Response(SocketType sock, const std::string &endpoint_name, const std::string &description)
```

---

## PerformanceLogger

**File:** `naming_backup\include\logging_utilities.hpp`  
**Line:** 258  

**Description:** /** * @brief Performance logging utility */

---

## Solution

**File:** `src\builtin_function_registry.cpp`  
**Line:** 1139  

### Methods

#### solve

```cpp
string solve()
```

/** * This function handles the task: )" + requirement + R"( */

---

## StepStatus

**File:** `naming_backup\include\sequential\workflow_engine.hpp`  
**Line:** 52  

**Description:** /** * @brief Workflow step execution status */

---

## ThreadPool

**File:** `naming_backup\kolosal-server\inference\src\inference.cpp`  
**Line:** 40  

### Methods

#### shutdown

```cpp
void shutdown()
```

Shutdown the thread pool

#### size

```cpp
size_t size() const
```

Return the number of active threads in the pool

#### worker

```cpp
void worker()
```

Worker function for each thread

---

## Tokenizer

**File:** `naming_backup\kolosal-server\inference\src\inference.cpp`  
**Line:** 197  

### Methods

#### tokenize

```cpp
vector<int32_t> tokenize(const std::string &text, bool add_bos = true)
```

New constructor takes shared pointers to model and context

#### detokenize

```cpp
string detokenize(const std::vector<int32_t> &tokens)
```

#### decode

```cpp
string decode(const int32_t &token)
```

#### applyTemplate

```cpp
string applyTemplate(std::vector<common_chat_msg> &messages, toolcall::client::ptr tc_client = nullptr)
```

#### shouldAddBos

```cpp
bool shouldAddBos() const
```

---

## UUIDGenerator

**File:** `naming_backup\include\agent\agent_data.hpp`  
**Line:** 147  

**Description:** /** * @brief Represents u u i d generator functionality */

### Methods

#### generate

```cpp
string generate()
```

---

## WorkflowAgentService

**File:** `naming_backup\include\workflow_agent_service.hpp`  
**Line:** 31  

**Description:** /** * @brief Provides workflow agent services */

### Methods

#### from_json

```cpp
void from_json(const json& j)
```

#### validate

```cpp
bool validate() const
```

#### to_json

```cpp
json to_json() const
```

#### from_json

```cpp
void from_json(const json& j)
```

#### validate

```cpp
bool validate() const
```

#### to_json

```cpp
json to_json() const
```

#### from_json

```cpp
void from_json(const json& j)
```

#### validate

```cpp
bool validate() const
```

#### from_json

```cpp
void from_json(const json& j)
```

#### validate

```cpp
bool validate() const
```

#### to_json

```cpp
json to_json() const
```

#### from_json

```cpp
void from_json(const json& j)
```

#### validate

```cpp
bool validate() const
```

#### to_json

```cpp
json to_json() const
```

#### create_Workflow

```cpp
future<WorkflowResponse> create_Workflow(const WorkflowRequest& request)
```

Core service methods

#### execute_Workflow

```cpp
future<WorkflowExecutionResponse> execute_Workflow(const WorkflowExecutionRequest& request)
```

Core service methods

#### getWorkflow_Status

```cpp
future<WorkflowResponse> getWorkflow_Status(const WorkflowStatusRequest& request)
```

Core service methods

#### list_Workflows

```cpp
future<json> list_Workflows()
```

Core service methods

#### delete_Workflow

```cpp
future<WorkflowResponse> delete_Workflow(const std::string& workflow_id)
```

#### execute_RAGWorkflow

```cpp
future<RAGWorkflowResponse> execute_RAGWorkflow(const RAGWorkflowRequest& request)
```

RAG workflow operations

#### search_RAGContext

```cpp
future<RAGWorkflowResponse> search_RAGContext(const RAGWorkflowRequest& request)
```

RAG workflow operations

#### create_Session

```cpp
future<SessionResponse> create_Session(const SessionRequest& request)
```

Session management

#### get_Session

```cpp
future<SessionResponse> get_Session(const std::string& session_id)
```

Session management

#### list_Sessions

```cpp
future<json> list_Sessions()
```

Session management

#### delete_Session

```cpp
future<SessionResponse> delete_Session(const std::string& session_id)
```

Session management

#### getSession_History

```cpp
future<json> getSession_History(const std::string& session_id)
```

#### createOrchestration_Plan

```cpp
future<json> createOrchestration_Plan(const json& request)
```

Orchestration operations

#### executeOrchestration_Plan

```cpp
future<json> executeOrchestration_Plan(const std::string& plan_id, const json& parameters)
```

Orchestration operations

#### getOrchestration_Status

```cpp
future<json> getOrchestration_Status(const std::string& plan_id)
```

Orchestration operations

#### generateWorkflow_Id

```cpp
string generateWorkflow_Id()
```

#### generateExecution_Id

```cpp
string generateExecution_Id()
```

#### generateSession_Id

```cpp
string generateSession_Id()
```

#### generateError_Message

```cpp
string generateError_Message(const std::string& operation, const std::exception& e)
```

---

## WorkflowStatus

**File:** `naming_backup\include\sequential\workflow_engine.hpp`  
**Line:** 39  

**Description:** /** * @brief Workflow execution status */

---

## WorkflowType

**File:** `naming_backup\include\sequential\workflow_engine.hpp`  
**Line:** 64  

**Description:** /** * @brief Workflow execution type */

---

