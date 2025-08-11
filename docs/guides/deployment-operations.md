# Deployment and Operations Guide

This guide provides comprehensive information for deploying, configuring, and operating the Kolosal Agent System v2.0 in various environments.

## Table of Contents

- [System Requirements](#system-requirements)
- [Installation Methods](#installation-methods)
- [Configuration Management](#configuration-management)
- [Deployment Architectures](#deployment-architectures)
- [Monitoring and Health Checks](#monitoring-and-health-checks)
- [Performance Optimization](#performance-optimization)
- [Security Considerations](#security-considerations)
- [Troubleshooting](#troubleshooting)
- [Backup and Recovery](#backup-and-recovery)

## System Requirements

### Minimum Requirements

**Hardware:**
- **CPU**: 4 cores minimum (8 cores recommended)
- **RAM**: 8 GB minimum (16 GB recommended for production)
- **Storage**: 20 GB free space (50 GB+ for production with models)
- **Network**: Stable internet connection for external services

**Software:**
- **OS**: Windows 10+, Ubuntu 20.04+, macOS 10.15+
- **CMake**: 3.14 or higher
- **C++ Compiler**: C++17 compatible (MSVC 2019+, GCC 9+, Clang 10+)
- **Git**: For repository management and updates

### Recommended Production Requirements

**Hardware:**
- **CPU**: 16+ cores with high clock speed
- **RAM**: 32 GB+ for handling multiple concurrent agents
- **Storage**: NVMe SSD with 100+ GB available
- **GPU**: Optional CUDA-compatible GPU for AI model acceleration
- **Network**: High-bandwidth connection with low latency

**Infrastructure:**
- **Load Balancer**: For distributing requests across instances
- **Monitoring**: Prometheus/Grafana for metrics
- **Logging**: Centralized logging system (ELK stack, etc.)
- **Backup**: Automated backup solution for configurations and data

## Installation Methods

### Method 1: Source Code Installation

```bash
# Clone the repository
git clone --recursive https://github.com/kolosalai/kolosal-agent.git
cd kolosal-agent

# Create build directory
mkdir build && cd build

# Configure with CMake (Debug)
cmake .. -DCMAKE_BUILD_TYPE=Debug \
         -DBUILD_TESTS=ON \
         -DBUILD_EXAMPLES=ON \
         -DMCP_PROTOCOL_ENABLED=ON

# Build the project
cmake --build . --config Debug --parallel

# Optional: Run tests
ctest --output-on-failure

# Optional: Install system-wide
sudo cmake --install . --config Debug
```

### Method 2: Docker Installation

```dockerfile
# Dockerfile
FROM ubuntu:22.04

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential cmake git \
    libcurl4-openssl-dev libyaml-cpp-dev \
    && rm -rf /var/lib/apt/lists/*

# Copy source code
COPY . /app
WORKDIR /app

# Build the application
RUN mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . --config Release

# Set up runtime
EXPOSE 8080 8081
CMD ["./build/kolosal-agent-unified", "--prod"]
```

```bash
# Build and run with Docker
docker build -t kolosal-agent:v2.0 .
docker run -d \
  --name kolosal-agent \
  -p 8080:8080 \
  -p 8081:8081 \
  -v $(pwd)/config:/app/config \
  -v $(pwd)/models:/app/models \
  -v $(pwd)/logs:/app/logs \
  kolosal-agent:v2.0
```

### Method 3: Docker Compose Installation

```yaml
# docker-compose.yml
version: '3.8'

services:
  kolosal-agent:
    build: .
    ports:
      - "8080:8080"  # LLM Server
      - "8081:8081"  # Agent API
    volumes:
      - ./config:/app/config
      - ./models:/app/models
      - ./logs:/app/logs
      - ./data:/app/data
    environment:
      - KOLOSAL_CONFIG=/app/config/production.yaml
      - KOLOSAL_LOG_LEVEL=INFO
      - KOLOSAL_ENABLE_METRICS=true
    restart: unless-stopped
    healthcheck:
      test: ["CMD", "curl", "-f", "http://localhost:8081/v1/health"]
      interval: 30s
      timeout: 10s
      retries: 3

  # Optional: Vector database
  qdrant:
    image: qdrant/qdrant:v1.7.4
    ports:
      - "6333:6333"
    volumes:
      - ./qdrant_storage:/qdrant/storage
    environment:
      - QDRANT_ALLOW_RECOVERY_MODE=true

  # Optional: Monitoring
  prometheus:
    image: prom/prometheus:v2.40.0
    ports:
      - "9090:9090"
    volumes:
      - ./monitoring/prometheus.yml:/etc/prometheus/prometheus.yml
    command:
      - '--config.file=/etc/prometheus/prometheus.yml'
      - '--web.console.libraries=/etc/prometheus/console_libraries'
      - '--web.console.templates=/etc/prometheus/consoles'

networks:
  default:
    driver: bridge
```

```bash
# Start all services
docker-compose up -d

# Check status
docker-compose ps

# View logs
docker-compose logs -f kolosal-agent

# Stop services
docker-compose down
```

## Configuration Management

### Configuration File Structure

```yaml
# config/production.yaml
system:
  name: "Kolosal Agent System v2.0"
  version: "2.0.0"
  environment: "production"
  
  server:
    host: "0.0.0.0"
    llm_port: 8080
    agent_api_port: 8081
    timeout: 60
    max_connections: 1000
    
  security:
    enable_cors: true
    allowed_origins: 
      - "https://yourdomain.com"
      - "https://api.yourdomain.com"
    enable_auth: true
    api_keys:
      - "sk-your-production-api-key-here"
    rate_limiting:
      enabled: true
      requests_per_minute: 1000
      burst_capacity: 100

  logging:
    level: "INFO"
    enable_console: true
    enable_file: true
    log_directory: "./logs"
    max_file_size_mb: 100
    max_files: 10
    enable_structured: true

  monitoring:
    enable_health_checks: true
    health_check_interval_seconds: 30
    enable_metrics: true
    metrics_port: 9091
    enable_performance_analytics: true
    enable_auto_recovery: true
    max_recovery_attempts: 3

  database:
    vector_db:
      type: "qdrant"
      host: "localhost"
      port: 6333
      collection_name: "kolosal_vectors"
    
    storage:
      type: "sqlite"
      path: "./data/kolosal.db"
      enable_wal: true

# Agent configurations
agents:
  - name: "system_coordinator"
    id: "coord-001"
    type: "coordinator"
    role: "COORDINATOR"
    priority: 1
    auto_start: true
    
    specializations:
      - "TASK_PLANNING"
      - "RESOURCE_MANAGEMENT"
      - "SYSTEM_MONITORING"
    
    capabilities:
      - "plan_execution"
      - "task_delegation"
      - "system_monitoring"
      - "resource_optimization"
    
    functions:
      - "plan_tasks"
      - "delegate_work"
      - "monitor_progress"
      - "optimize_resources"
    
    config:
      max_concurrent_tasks: 10
      memory_limit_mb: 1024
      enable_persistence: true
      heartbeat_interval_seconds: 10
      
  - name: "data_analyst"
    id: "analyst-001"
    type: "analyst"
    role: "ANALYST"
    priority: 2
    auto_start: true
    
    specializations:
      - "DATA_ANALYSIS"
      - "STATISTICAL_PROCESSING"
      - "REPORT_GENERATION"
    
    capabilities:
      - "data_processing"
      - "statistical_analysis"
      - "report_generation"
      - "visualization"
    
    functions:
      - "analyze_data"
      - "generate_report"
      - "process_statistics"
      - "create_visualization"
    
    config:
      max_concurrent_tasks: 5
      memory_limit_mb: 2048
      enable_persistence: true
      specialized_tools:
        - "statistical_analyzer"
        - "data_visualizer"

# External service configurations
services:
  web_search:
    enabled: true
    provider: "serpapi"
    api_key: "${WEB_SEARCH_API_KEY}"
    max_results: 10
    
  document_processing:
    enabled: true
    max_file_size_mb: 50
    supported_formats: ["pdf", "docx", "html", "txt"]
    
  llm_models:
    default_model: "qwen3-0.6b"
    model_path: "./models"
    enable_gpu: true
    max_tokens: 4096
```

### Environment Variables

```bash
# Core configuration
export KOLOSAL_CONFIG="./config/production.yaml"
export KOLOSAL_LOG_LEVEL="INFO"
export KOLOSAL_ENVIRONMENT="production"

# Server configuration
export KOLOSAL_HOST="0.0.0.0"
export KOLOSAL_LLM_PORT="8080"
export KOLOSAL_API_PORT="8081"

# Database configuration
export KOLOSAL_VECTOR_DB_HOST="localhost"
export KOLOSAL_VECTOR_DB_PORT="6333"
export KOLOSAL_DB_PATH="./data/kolosal.db"

# Security configuration
export KOLOSAL_API_KEY="sk-your-secure-api-key"
export KOLOSAL_ENABLE_AUTH="true"
export KOLOSAL_ENABLE_CORS="true"

# External services
export WEB_SEARCH_API_KEY="your-web-search-api-key"
export OPENAI_API_KEY="your-openai-api-key"  # If using OpenAI models

# Monitoring
export KOLOSAL_ENABLE_METRICS="true"
export KOLOSAL_METRICS_PORT="9091"
```

### Configuration Validation

```bash
# Validate configuration before starting
./kolosal-agent-unified --config ./config/production.yaml --validate-only

# Check configuration syntax
yamllint ./config/production.yaml

# Test configuration with dry run
./kolosal-agent-unified --config ./config/production.yaml --dry-run
```

## Deployment Architectures

### Single Instance Deployment

```
┌─────────────────────────────────────────┐
│            Load Balancer                │
├─────────────────────────────────────────┤
│         Kolosal Agent System            │
│    ┌─────────────┬─────────────────┐    │
│    │ LLM Server  │   Agent API     │    │
│    │   :8080     │     :8081       │    │
│    └─────────────┴─────────────────┘    │
├─────────────────────────────────────────┤
│            Vector Database              │
│               (Qdrant)                  │
└─────────────────────────────────────────┘
```

**Use Cases:** Development, testing, small-scale production

**Configuration:**
```yaml
system:
  server:
    host: "0.0.0.0"
    llm_port: 8080
    agent_api_port: 8081
  
  deployment:
    mode: "single_instance"
    max_agents: 20
    max_concurrent_requests: 100
```

### High Availability Deployment

```
┌─────────────────────────────────────────────────────────┐
│                   Load Balancer                         │
│                (HAProxy/NGINX)                          │
├─────────────────┬─────────────────┬─────────────────────┤
│   Instance 1    │   Instance 2    │    Instance 3       │
│ ┌─────┬───────┐ │ ┌─────┬───────┐ │ ┌─────┬───────┐     │
│ │ LLM │ API   │ │ │ LLM │ API   │ │ │ LLM │ API   │     │
│ │8080 │ 8081  │ │ │8080 │ 8081  │ │ │8080 │ 8081  │     │
│ └─────┴───────┘ │ └─────┴───────┘ │ └─────┴───────┘     │
├─────────────────┼─────────────────┼─────────────────────┤
│         Shared Storage & Databases                      │
│  ┌─────────────┬─────────────┬─────────────────────┐   │
│  │Vector DB    │ Message     │    File Storage     │   │
│  │ (Qdrant)    │ Queue       │    (NFS/S3)         │   │
│  │ Cluster     │ (Redis)     │                     │   │
│  └─────────────┴─────────────┴─────────────────────┘   │
└─────────────────────────────────────────────────────────┘
```

**Use Cases:** Production environments requiring high availability

**Configuration:**
```yaml
system:
  deployment:
    mode: "high_availability"
    instances: 3
    load_balancer:
      type: "round_robin"
      health_check_path: "/v1/health"
      health_check_interval: 10
    
  clustering:
    enabled: true
    node_id: "${NODE_ID}"
    cluster_nodes:
      - "node1.internal:8081"
      - "node2.internal:8081"
      - "node3.internal:8081"
    
  shared_storage:
    type: "nfs"
    mount_point: "/shared/kolosal"
    
  message_queue:
    type: "redis"
    host: "redis.internal"
    port: 6379
    cluster: true
```

### Kubernetes Deployment

```yaml
# kubernetes/namespace.yaml
apiVersion: v1
kind: Namespace
metadata:
  name: kolosal-agent

---
# kubernetes/configmap.yaml
apiVersion: v1
kind: ConfigMap
metadata:
  name: kolosal-config
  namespace: kolosal-agent
data:
  production.yaml: |
    system:
      name: "Kolosal Agent System v2.0"
      environment: "kubernetes"
      server:
        host: "0.0.0.0"
        llm_port: 8080
        agent_api_port: 8081

---
# kubernetes/deployment.yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: kolosal-agent
  namespace: kolosal-agent
  labels:
    app: kolosal-agent
    version: v2.0
spec:
  replicas: 3
  strategy:
    type: RollingUpdate
    rollingUpdate:
      maxUnavailable: 1
      maxSurge: 1
  selector:
    matchLabels:
      app: kolosal-agent
  template:
    metadata:
      labels:
        app: kolosal-agent
        version: v2.0
    spec:
      containers:
      - name: kolosal-agent
        image: kolosal/agent-system:v2.0
        ports:
        - name: llm-port
          containerPort: 8080
        - name: api-port
          containerPort: 8081
        - name: metrics-port
          containerPort: 9091
        
        resources:
          requests:
            cpu: 1000m
            memory: 2Gi
          limits:
            cpu: 4000m
            memory: 8Gi
        
        env:
        - name: KOLOSAL_CONFIG
          value: "/config/production.yaml"
        - name: KOLOSAL_ENVIRONMENT
          value: "kubernetes"
        - name: NODE_ID
          valueFrom:
            fieldRef:
              fieldPath: metadata.name
        
        volumeMounts:
        - name: config-volume
          mountPath: /config
        - name: models-volume
          mountPath: /app/models
        - name: data-volume
          mountPath: /app/data
        
        livenessProbe:
          httpGet:
            path: /v1/health
            port: api-port
          initialDelaySeconds: 30
          periodSeconds: 10
          timeoutSeconds: 5
          failureThreshold: 3
        
        readinessProbe:
          httpGet:
            path: /v1/system/status
            port: api-port
          initialDelaySeconds: 15
          periodSeconds: 5
          timeoutSeconds: 3
          failureThreshold: 2
      
      volumes:
      - name: config-volume
        configMap:
          name: kolosal-config
      - name: models-volume
        persistentVolumeClaim:
          claimName: kolosal-models-pvc
      - name: data-volume
        persistentVolumeClaim:
          claimName: kolosal-data-pvc

---
# kubernetes/service.yaml
apiVersion: v1
kind: Service
metadata:
  name: kolosal-agent-service
  namespace: kolosal-agent
  labels:
    app: kolosal-agent
spec:
  type: ClusterIP
  ports:
  - name: llm-port
    port: 8080
    targetPort: llm-port
    protocol: TCP
  - name: api-port
    port: 8081
    targetPort: api-port
    protocol: TCP
  - name: metrics-port
    port: 9091
    targetPort: metrics-port
    protocol: TCP
  selector:
    app: kolosal-agent

---
# kubernetes/ingress.yaml
apiVersion: networking.k8s.io/v1
kind: Ingress
metadata:
  name: kolosal-agent-ingress
  namespace: kolosal-agent
  annotations:
    nginx.ingress.kubernetes.io/rewrite-target: /
    nginx.ingress.kubernetes.io/ssl-redirect: "true"
    cert-manager.io/cluster-issuer: "letsencrypt-prod"
spec:
  tls:
  - hosts:
    - kolosal-api.yourdomain.com
    secretName: kolosal-tls-secret
  rules:
  - host: kolosal-api.yourdomain.com
    http:
      paths:
      - path: /
        pathType: Prefix
        backend:
          service:
            name: kolosal-agent-service
            port:
              number: 8081

---
# kubernetes/hpa.yaml
apiVersion: autoscaling/v2
kind: HorizontalPodAutoscaler
metadata:
  name: kolosal-agent-hpa
  namespace: kolosal-agent
spec:
  scaleTargetRef:
    apiVersion: apps/v1
    kind: Deployment
    name: kolosal-agent
  minReplicas: 2
  maxReplicas: 10
  metrics:
  - type: Resource
    resource:
      name: cpu
      target:
        type: Utilization
        averageUtilization: 70
  - type: Resource
    resource:
      name: memory
      target:
        type: Utilization
        averageUtilization: 80

---
# kubernetes/pvc.yaml
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: kolosal-models-pvc
  namespace: kolosal-agent
spec:
  accessModes:
    - ReadWriteMany
  resources:
    requests:
      storage: 50Gi
  storageClassName: fast-ssd

---
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: kolosal-data-pvc
  namespace: kolosal-agent
spec:
  accessModes:
    - ReadWriteMany
  resources:
    requests:
      storage: 20Gi
  storageClassName: fast-ssd
```

```bash
# Deploy to Kubernetes
kubectl apply -f kubernetes/

# Check deployment status
kubectl get pods -n kolosal-agent
kubectl get services -n kolosal-agent
kubectl get ingress -n kolosal-agent

# Scale deployment
kubectl scale deployment kolosal-agent --replicas=5 -n kolosal-agent

# View logs
kubectl logs -f deployment/kolosal-agent -n kolosal-agent

# Update deployment
kubectl set image deployment/kolosal-agent kolosal-agent=kolosal/agent-system:v2.1 -n kolosal-agent
```

## Monitoring and Health Checks

### Health Check Endpoints

```bash
# Basic health check
curl http://localhost:8081/v1/health

# Detailed system status
curl http://localhost:8081/v1/system/status

# Metrics endpoint (Prometheus format)
curl http://localhost:9091/metrics
```

### Monitoring Configuration

```yaml
# monitoring/prometheus.yml
global:
  scrape_interval: 15s
  evaluation_interval: 15s

rule_files:
  - "kolosal_rules.yml"

scrape_configs:
  - job_name: 'kolosal-agent'
    static_configs:
      - targets: ['localhost:9091']
    scrape_interval: 10s
    metrics_path: /metrics
    
  - job_name: 'kolosal-agent-k8s'
    kubernetes_sd_configs:
      - role: pod
        namespaces:
          names:
            - kolosal-agent
    relabel_configs:
      - source_labels: [__meta_kubernetes_pod_label_app]
        action: keep
        regex: kolosal-agent
      - source_labels: [__meta_kubernetes_pod_container_port_name]
        action: keep
        regex: metrics-port

alerting:
  alertmanagers:
    - static_configs:
        - targets:
          - alertmanager:9093
```

### Alert Rules

```yaml
# monitoring/kolosal_rules.yml
groups:
  - name: kolosal-agent-alerts
    rules:
      - alert: KolosalAgentDown
        expr: up{job="kolosal-agent"} == 0
        for: 1m
        labels:
          severity: critical
        annotations:
          summary: "Kolosal Agent instance is down"
          description: "Instance {{ $labels.instance }} has been down for more than 1 minute."

      - alert: HighCPUUsage
        expr: kolosal_cpu_usage_percent > 80
        for: 5m
        labels:
          severity: warning
        annotations:
          summary: "High CPU usage detected"
          description: "CPU usage is {{ $value }}% on instance {{ $labels.instance }}"

      - alert: HighMemoryUsage
        expr: kolosal_memory_usage_percent > 85
        for: 5m
        labels:
          severity: warning
        annotations:
          summary: "High memory usage detected"
          description: "Memory usage is {{ $value }}% on instance {{ $labels.instance }}"

      - alert: AgentFailureRate
        expr: rate(kolosal_agent_failures_total[5m]) > 0.1
        for: 2m
        labels:
          severity: critical
        annotations:
          summary: "High agent failure rate"
          description: "Agent failure rate is {{ $value }} failures per second"

      - alert: LowHealthScore
        expr: kolosal_system_health_score < 0.8
        for: 3m
        labels:
          severity: warning
        annotations:
          summary: "System health score is low"
          description: "System health score is {{ $value }} on instance {{ $labels.instance }}"
```

### Grafana Dashboards

```json
{
  "dashboard": {
    "id": null,
    "title": "Kolosal Agent System v2.0",
    "tags": ["kolosal", "agents", "ai"],
    "timezone": "browser",
    "panels": [
      {
        "id": 1,
        "title": "System Overview",
        "type": "stat",
        "targets": [
          {
            "expr": "kolosal_system_health_score",
            "legendFormat": "Health Score"
          },
          {
            "expr": "kolosal_active_agents_total",
            "legendFormat": "Active Agents"
          },
          {
            "expr": "rate(kolosal_requests_total[5m])",
            "legendFormat": "Requests/sec"
          }
        ]
      },
      {
        "id": 2,
        "title": "Agent Performance",
        "type": "graph",
        "targets": [
          {
            "expr": "kolosal_agent_execution_duration_seconds",
            "legendFormat": "Execution Time - {{ agent_id }}"
          },
          {
            "expr": "rate(kolosal_agent_executions_total[5m])",
            "legendFormat": "Executions/sec - {{ agent_id }}"
          }
        ]
      },
      {
        "id": 3,
        "title": "Resource Usage",
        "type": "graph",
        "targets": [
          {
            "expr": "kolosal_cpu_usage_percent",
            "legendFormat": "CPU Usage %"
          },
          {
            "expr": "kolosal_memory_usage_percent",
            "legendFormat": "Memory Usage %"
          }
        ]
      }
    ]
  }
}
```

## Performance Optimization

### System Optimization

```yaml
# config/performance.yaml
system:
  performance:
    # Thread pool configuration
    thread_pools:
      agent_execution:
        size: 16
        queue_size: 1000
      workflow_execution:
        size: 8
        queue_size: 500
      io_operations:
        size: 4
        queue_size: 200
    
    # Memory management
    memory:
      agent_cache_size: 1000
      function_cache_size: 500
      result_cache_size: 2000
      cache_ttl_seconds: 3600
    
    # Connection pooling
    connections:
      database_pool_size: 20
      http_client_pool_size: 10
      vector_db_pool_size: 5
    
    # Request handling
    request_handling:
      max_concurrent_requests: 1000
      request_timeout_seconds: 300
      enable_request_batching: true
      batch_size: 10
      batch_timeout_ms: 100

  optimization:
    # Agent optimization
    agents:
      enable_lazy_loading: true
      preload_critical_agents: true
      agent_hibernation_timeout: 300
      
    # Function optimization
    functions:
      enable_function_caching: true
      cache_compiled_functions: true
      optimize_function_calls: true
      
    # Workflow optimization
    workflows:
      enable_parallel_execution: true
      max_parallel_steps: 5
      optimize_step_ordering: true
```

### Database Optimization

```yaml
# Vector database optimization
vector_db:
  type: "qdrant"
  config:
    # Performance settings
    max_connections: 20
    connection_timeout: 10
    request_timeout: 30
    
    # Index optimization
    index_settings:
      ef_construct: 200
      ef_search: 100
      m_connections: 16
      
    # Memory optimization
    memory_map_threshold: "2GB"
    segment_size: "100MB"
    optimize_for: "speed"  # or "memory"
    
    # Clustering settings
    clustering:
      enabled: true
      replication_factor: 2
      sharding_key: "agent_id"

# SQLite optimization
sqlite:
  config:
    # Performance settings
    cache_size: "100MB"
    journal_mode: "WAL"
    synchronous: "NORMAL"
    temp_store: "MEMORY"
    
    # Connection settings
    max_connections: 50
    connection_timeout: 30
    
    # Optimization
    auto_vacuum: "INCREMENTAL"
    page_size: 4096
    enable_fts: true
```

### Application Optimization

```cpp
// C++ optimization flags
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DENABLE_NATIVE_OPTS=ON \
         -DENABLE_LTO=ON \
         -DENABLE_FAST_MATH=ON \
         -DUSE_JEMALLOC=ON \
         -DENABLE_CUDA=ON
```

```bash
# Runtime optimization environment variables
export KOLOSAL_OPTIMIZE_FOR_THROUGHPUT=true
export KOLOSAL_ENABLE_FUNCTION_CACHING=true
export KOLOSAL_PARALLEL_AGENT_EXECUTION=true
export KOLOSAL_MEMORY_POOL_SIZE=2048

# NUMA optimization (Linux)
numactl --interleave=all ./kolosal-agent-unified

# CPU affinity (Linux)
taskset -c 0-15 ./kolosal-agent-unified

# Process priority
nice -n -10 ./kolosal-agent-unified
```

## Security Considerations

### Authentication and Authorization

```yaml
# Security configuration
security:
  authentication:
    # API Key authentication
    api_key:
      enabled: true
      required: true
      header_name: "X-API-Key"
      keys:
        - name: "production_key"
          key: "${KOLOSAL_API_KEY}"
          permissions: ["read", "write", "admin"]
        - name: "readonly_key"
          key: "${KOLOSAL_READONLY_KEY}"
          permissions: ["read"]
    
    # JWT authentication
    jwt:
      enabled: false
      secret: "${JWT_SECRET}"
      expiration_hours: 24
      issuer: "kolosal-agent-system"
      
  authorization:
    # Role-based access control
    roles:
      admin:
        permissions: ["*"]
      user:
        permissions: ["agents:read", "agents:execute", "system:status"]
      readonly:
        permissions: ["agents:read", "system:status"]
    
    # Resource-based permissions
    resources:
      agents:
        create: ["admin"]
        read: ["admin", "user", "readonly"]
        update: ["admin"]
        delete: ["admin"]
        execute: ["admin", "user"]
      system:
        status: ["admin", "user", "readonly"]
        reload: ["admin"]
        metrics: ["admin"]

  # Network security
  network:
    # CORS settings
    cors:
      enabled: true
      allowed_origins:
        - "https://yourdomain.com"
        - "https://*.yourdomain.com"
      allowed_methods: ["GET", "POST", "PUT", "DELETE"]
      allowed_headers: ["Content-Type", "Authorization", "X-API-Key"]
      max_age: 86400
    
    # Rate limiting
    rate_limiting:
      enabled: true
      global_limit: 1000  # requests per minute
      per_ip_limit: 100   # requests per minute per IP
      burst_capacity: 50
      
    # SSL/TLS
    tls:
      enabled: true
      cert_file: "/etc/ssl/certs/kolosal.crt"
      key_file: "/etc/ssl/private/kolosal.key"
      min_version: "1.2"
      
  # Input validation
  validation:
    # Request validation
    max_request_size: "10MB"
    max_json_depth: 10
    allow_null_values: false
    
    # Parameter validation
    strict_type_checking: true
    validate_json_schema: true
    sanitize_inputs: true
    
  # Security headers
  headers:
    enable_hsts: true
    enable_csp: true
    csp_policy: "default-src 'self'; script-src 'self' 'unsafe-inline'"
    enable_xss_protection: true
    enable_content_type_options: true
    enable_frame_options: true
```

### Encryption and Data Protection

```yaml
# Encryption settings
encryption:
  # Data at rest
  data_at_rest:
    enabled: true
    algorithm: "AES-256-GCM"
    key_source: "vault"  # or "file", "env"
    key_rotation_days: 90
    
    # Database encryption
    database:
      encrypt_sensitive_fields: true
      fields: ["agent_memory", "function_parameters", "results"]
    
  # Data in transit
  data_in_transit:
    # Internal communication encryption
    internal_tls: true
    client_cert_required: false
    
    # External API encryption
    force_https: true
    min_tls_version: "1.2"
    cipher_suites:
      - "TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384"
      - "TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256"

# Secrets management
secrets:
  vault:
    enabled: true
    url: "https://vault.internal:8200"
    auth_method: "kubernetes"
    role: "kolosal-agent"
    secret_path: "secret/kolosal"
  
  # Secret rotation
  rotation:
    api_keys:
      enabled: true
      interval_days: 30
    certificates:
      enabled: true
      interval_days: 90
```

### Security Monitoring

```yaml
# Security monitoring
monitoring:
  security:
    # Audit logging
    audit:
      enabled: true
      log_level: "INFO"
      log_file: "/var/log/kolosal/audit.log"
      events:
        - "authentication"
        - "authorization" 
        - "agent_creation"
        - "agent_deletion"
        - "configuration_changes"
        - "system_access"
    
    # Intrusion detection
    intrusion_detection:
      enabled: true
      failed_auth_threshold: 5
      failed_auth_window_minutes: 10
      suspicious_pattern_detection: true
      
    # Anomaly detection
    anomaly_detection:
      enabled: true
      baseline_learning_days: 7
      deviation_threshold: 2.5
      alerts:
        - "unusual_request_patterns"
        - "unexpected_agent_behavior"
        - "resource_usage_anomalies"
```

## Troubleshooting

### Common Issues and Solutions

#### 1. Service Start Issues

**Issue**: Server fails to start with port binding error
```bash
Error: Failed to bind to port 8081: Address already in use
```

**Solution**:
```bash
# Check what's using the port
netstat -tulpn | grep :8081
# or
lsof -i :8081

# Kill the process using the port
sudo kill -9 <PID>

# Or use a different port
./kolosal-agent-unified --config config.yaml --agent-api-port 8082
```

#### 2. Agent Creation Failures

**Issue**: Agent creation fails with configuration errors
```bash
Error: Failed to create agent: Invalid configuration - missing required field 'type'
```

**Solution**:
```bash
# Validate configuration file
./kolosal-agent-unified --config config.yaml --validate-only

# Check configuration syntax
yamllint config.yaml

# Use minimal working configuration
curl -X POST http://localhost:8081/v1/agents \
  -H "Content-Type: application/json" \
  -d '{
    "name": "test_agent",
    "type": "specialist",
    "role": 2
  }'
```

#### 3. Memory Issues

**Issue**: System runs out of memory or becomes slow
```bash
Error: std::bad_alloc - insufficient memory
```

**Solution**:
```yaml
# Optimize memory settings in config
system:
  memory:
    agent_cache_size: 100  # Reduce from default
    max_agents: 10         # Limit concurrent agents
    garbage_collection:
      enabled: true
      interval_seconds: 60
      
agents:
  - config:
      memory_limit_mb: 512  # Limit per agent
```

#### 4. Database Connection Issues

**Issue**: Vector database connection failures
```bash
Error: Failed to connect to vector database: Connection refused
```

**Solution**:
```bash
# Check database status
docker ps | grep qdrant

# Start vector database
docker run -d -p 6333:6333 qdrant/qdrant:v1.7.4

# Test connection
curl http://localhost:6333/health

# Update configuration
# In config.yaml:
database:
  vector_db:
    host: "localhost"  # Ensure correct host
    port: 6333         # Ensure correct port
    timeout: 30        # Increase timeout
```

#### 5. Performance Issues

**Issue**: Slow response times and high latency
```bash
Warning: Agent execution took 15.2s (expected < 5s)
```

**Solution**:
```yaml
# Enable performance optimizations
system:
  performance:
    thread_pools:
      agent_execution:
        size: 16  # Increase thread pool
    memory:
      function_cache_size: 1000  # Increase cache
    optimization:
      agents:
        enable_lazy_loading: false  # Preload for speed
        preload_critical_agents: true
```

### Debugging Tools

#### 1. Log Analysis

```bash
# Enable debug logging
export KOLOSAL_LOG_LEVEL=DEBUG
./kolosal-agent-unified --verbose

# Analyze logs
tail -f logs/kolosal-agent.log | grep ERROR
grep -E "(ERROR|WARN)" logs/kolosal-agent.log | tail -50

# Structured log analysis with jq (if using JSON logging)
tail -f logs/kolosal-agent.log | jq 'select(.level == "ERROR")'
```

#### 2. Health Diagnostics

```bash
# Comprehensive health check
curl -s http://localhost:8081/v1/health | jq .

# System diagnostics
curl -s http://localhost:8081/v1/system/status | jq .

# Agent-specific diagnostics
curl -s http://localhost:8081/v1/agents/agent-id/status | jq .
```

#### 3. Performance Profiling

```bash
# CPU profiling (Linux)
perf record -g ./kolosal-agent-unified
perf report

# Memory profiling with Valgrind
valgrind --tool=memcheck --leak-check=full ./kolosal-agent-unified

# Memory profiling with AddressSanitizer
cmake .. -DENABLE_ASAN=ON
./kolosal-agent-unified
```

#### 4. Network Diagnostics

```bash
# Test API endpoints
curl -v http://localhost:8081/v1/health

# Check port accessibility
telnet localhost 8081

# Monitor network connections
netstat -tulpn | grep kolosal
ss -tulpn | grep kolosal
```

### Log Analysis and Debugging

#### Log File Locations

```bash
# Default log locations
./logs/kolosal-agent.log          # Main application log
./logs/kolosal-agent-error.log    # Error log
./logs/kolosal-agent-access.log   # HTTP access log
./logs/kolosal-agent-audit.log    # Security audit log
./logs/agents/                    # Agent-specific logs
./logs/performance/               # Performance metrics
```

#### Log Format Examples

```json
// JSON structured logging format
{
  "timestamp": "2025-08-12T10:30:45.123Z",
  "level": "INFO",
  "component": "AgentService",
  "agent_id": "analyst-001",
  "message": "Agent execution completed successfully",
  "execution_time_ms": 245,
  "function": "analyze_data",
  "correlation_id": "req-abc123"
}

// Error log format
{
  "timestamp": "2025-08-12T10:30:45.123Z",
  "level": "ERROR", 
  "component": "AgentCore",
  "agent_id": "coord-001",
  "error": "Function execution failed",
  "error_code": "FUNC_EXEC_ERROR",
  "stack_trace": "...",
  "correlation_id": "req-def456"
}
```

#### Log Analysis Scripts

```bash
#!/bin/bash
# analyze_logs.sh - Log analysis script

LOG_FILE="logs/kolosal-agent.log"

echo "=== Kolosal Agent Log Analysis ==="
echo "Log file: $LOG_FILE"
echo "Analysis time: $(date)"
echo

echo "=== Error Summary ==="
grep "ERROR" $LOG_FILE | tail -20

echo
echo "=== Performance Issues ==="
grep "execution_time_ms" $LOG_FILE | \
  jq 'select(.execution_time_ms > 5000)' | \
  head -10

echo
echo "=== Agent Activity ==="
grep "agent_id" $LOG_FILE | \
  jq -r '.agent_id' | \
  sort | uniq -c | sort -nr

echo
echo "=== Most Common Errors ==="
grep "ERROR" $LOG_FILE | \
  jq -r '.error_code // .error' | \
  sort | uniq -c | sort -nr | head -10
```

## Backup and Recovery

### Backup Strategy

```bash
#!/bin/bash
# backup.sh - Comprehensive backup script

BACKUP_DIR="/backups/kolosal/$(date +%Y%m%d_%H%M%S)"
mkdir -p $BACKUP_DIR

echo "Creating backup at: $BACKUP_DIR"

# Configuration backup
echo "Backing up configuration..."
cp -r config/ $BACKUP_DIR/config/

# Data backup
echo "Backing up data..."
cp -r data/ $BACKUP_DIR/data/

# Model backup (if local models)
echo "Backing up models..."
cp -r models/ $BACKUP_DIR/models/

# Logs backup (last 7 days)
echo "Backing up logs..."
find logs/ -name "*.log" -mtime -7 -exec cp {} $BACKUP_DIR/logs/ \;

# Database backup
echo "Backing up databases..."
sqlite3 data/kolosal.db ".backup $BACKUP_DIR/kolosal.db"

# Vector database backup (Qdrant)
if command -v curl >/dev/null; then
  echo "Backing up vector database..."
  curl -X POST "http://localhost:6333/collections/kolosal_vectors/snapshots" \
    -H "Content-Type: application/json" \
    -d '{"name": "backup_'$(date +%Y%m%d_%H%M%S)'"}'
fi

# Create archive
echo "Creating archive..."
tar -czf "$BACKUP_DIR.tar.gz" -C "$(dirname $BACKUP_DIR)" "$(basename $BACKUP_DIR)"
rm -rf $BACKUP_DIR

echo "Backup completed: $BACKUP_DIR.tar.gz"

# Cleanup old backups (keep last 30 days)
find /backups/kolosal/ -name "*.tar.gz" -mtime +30 -delete

echo "Backup process finished"
```

### Recovery Procedures

```bash
#!/bin/bash
# restore.sh - System restore script

BACKUP_FILE=$1
RESTORE_DIR="/tmp/kolosal_restore"

if [ -z "$BACKUP_FILE" ]; then
  echo "Usage: $0 <backup_file.tar.gz>"
  exit 1
fi

echo "Restoring from: $BACKUP_FILE"

# Stop the service
echo "Stopping Kolosal Agent service..."
sudo systemctl stop kolosal-agent

# Extract backup
echo "Extracting backup..."
mkdir -p $RESTORE_DIR
tar -xzf $BACKUP_FILE -C $RESTORE_DIR

# Restore configuration
echo "Restoring configuration..."
cp -r $RESTORE_DIR/*/config/* config/

# Restore data
echo "Restoring data..."
cp -r $RESTORE_DIR/*/data/* data/

# Restore database
echo "Restoring database..."
cp $RESTORE_DIR/*/kolosal.db data/kolosal.db

# Restore models if needed
if [ -d "$RESTORE_DIR/*/models" ]; then
  echo "Restoring models..."
  cp -r $RESTORE_DIR/*/models/* models/
fi

# Set correct permissions
chown -R kolosal:kolosal config/ data/ models/ logs/
chmod -R 750 config/ data/

# Validate configuration
echo "Validating configuration..."
./kolosal-agent-unified --config config/production.yaml --validate-only

if [ $? -eq 0 ]; then
  echo "Configuration validation passed"
  
  # Start the service
  echo "Starting Kolosal Agent service..."
  sudo systemctl start kolosal-agent
  
  # Wait for service to be ready
  sleep 10
  
  # Verify service is running
  curl -f http://localhost:8081/v1/health >/dev/null 2>&1
  
  if [ $? -eq 0 ]; then
    echo "Restore completed successfully"
  else
    echo "Service started but health check failed"
    exit 1
  fi
else
  echo "Configuration validation failed"
  exit 1
fi

# Cleanup
rm -rf $RESTORE_DIR
echo "Restore process finished"
```

### Disaster Recovery

```yaml
# disaster-recovery.yaml
disaster_recovery:
  # Recovery Time Objective (RTO)
  rto_minutes: 30
  
  # Recovery Point Objective (RPO)  
  rpo_minutes: 15
  
  # Backup frequency
  backup_schedule:
    full_backup: "0 2 * * 0"      # Weekly full backup
    incremental: "0 */6 * * *"    # Every 6 hours
    configuration: "0 * * * *"    # Hourly config backup
    
  # Monitoring
  monitoring:
    backup_alerts: true
    recovery_testing: true
    test_frequency_days: 30
    
  # Failover procedures
  failover:
    automatic: true
    health_check_failures: 3
    failover_timeout_minutes: 5
    
  # Multi-region setup
  regions:
    primary: "us-east-1"
    secondary: "us-west-2"
    replication_delay_seconds: 60
```

This comprehensive deployment and operations guide provides the foundation for successfully deploying, monitoring, and maintaining the Kolosal Agent System v2.0 in production environments.
