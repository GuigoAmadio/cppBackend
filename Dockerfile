# ========================================
# DOCKERFILE - C++ BACKEND
# Multi-stage build for production
# ========================================

# ===== STAGE 1: BUILDER =====
FROM ubuntu:22.04 AS builder

# Evitar prompts interativos
ENV DEBIAN_FRONTEND=noninteractive

# Instalar dependências de build
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libpq-dev \
    libssl-dev \
    libhiredis-dev \
    git \
    && rm -rf /var/lib/apt/lists/*

# Configurar diretório de trabalho
WORKDIR /app

# Copiar apenas os arquivos necessários para build
COPY CMakeLists.txt ./
COPY src/ ./src/
COPY include/ ./include/
COPY lib/ ./lib/

# Build do projeto
RUN mkdir -p build && cd build && \
    cmake .. && \
    make -j$(nproc) && \
    strip cppBackend

# ===== STAGE 2: PRODUCTION =====
FROM ubuntu:22.04

# Evitar prompts interativos
ENV DEBIAN_FRONTEND=noninteractive

# Instalar apenas dependências de runtime
RUN apt-get update && apt-get install -y \
    libpq5 \
    libssl3 \
    libhiredis0.14 \
    ca-certificates \
    curl \
    && rm -rf /var/lib/apt/lists/*

# Criar usuário não-root para segurança
RUN useradd -m -u 1000 -s /bin/bash appuser

# Configurar diretório de trabalho
WORKDIR /app

# Copiar binário do builder
COPY --from=builder /app/build/cppBackend /app/cppBackend
COPY --from=builder /app/lib/ /app/lib/

# Copiar arquivos de configuração (se existirem)
# COPY config/ /app/config/

# Criar diretórios necessários
RUN mkdir -p /app/logs /app/exports && \
    chown -R appuser:appuser /app

# Trocar para usuário não-root
USER appuser

# Expor porta
EXPOSE 8080

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD curl -f http://localhost:8080/health || exit 1

# Comando padrão
CMD ["./cppBackend"]

