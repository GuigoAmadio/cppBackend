# 🐘 Como Consertar PostgreSQL no C++ Backend

## 🔍 **Problema**

Erro de compilação ao tentar usar PostgreSQL:

```
error: 'at_quick_exit' has not been declared in '::'
```

**Causa:** Mistura de compiladores (UCRT64 vs MINGW64)

---

## ✅ **Solução 1: Instalar PostgreSQL para UCRT64 (RECOMENDADO)**

### **Passo 1: Remover PostgreSQL MINGW64**

```bash
C:\msys64\usr\bin\bash.exe -lc "pacman -R mingw-w64-x86_64-postgresql --noconfirm"
```

### **Passo 2: Instalar PostgreSQL UCRT64**

```bash
C:\msys64\usr\bin\bash.exe -lc "pacman -S mingw-w64-ucrt-x86_64-postgresql --noconfirm"
```

### **Passo 3: Atualizar CMakeLists.txt**

Mude os caminhos:

```cmake
# PostgreSQL
find_library(LIBPQ_LIBRARY NAMES pq libpq PATHS
    "C:/msys64/ucrt64/lib"      # <- MUDOU de mingw64 para ucrt64
    NO_DEFAULT_PATH
)
find_path(LIBPQ_INCLUDE_DIR NAMES libpq-fe.h PATHS
    "C:/msys64/ucrt64/include"  # <- MUDOU de mingw64 para ucrt64
    NO_DEFAULT_PATH
)
```

### **Passo 4: Recompilar**

```bash
cd build
rm -r *
cmake -G "MinGW Makefiles" ..
mingw32-make
```

---

## ✅ **Solução 2: Mudar Projeto para MINGW64**

Se preferir manter PostgreSQL MINGW64, mude o compilador do projeto:

### **Passo 1: Limpar build**

```bash
cd build
rm -r *
```

### **Passo 2: Configurar para MINGW64**

```bash
$env:Path = "C:\msys64\mingw64\bin;$env:Path"
cmake -G "MinGW Makefiles" -DCMAKE_CXX_COMPILER="C:/msys64/mingw64/bin/g++.exe" ..
mingw32-make
```

---

## ✅ **Solução 3: Usar Docker (Mais Simples)**

Para evitar problemas de compilador, use PostgreSQL em Docker:

### **Passo 1: Criar docker-compose.yml**

```yaml
version: "3.8"
services:
  postgres:
    image: postgres:16-alpine
    environment:
      POSTGRES_USER: cppuser
      POSTGRES_PASSWORD: cpppass
      POSTGRES_DB: cppbackend
    ports:
      - "5432:5432"
    volumes:
      - postgres_data:/var/lib/postgresql/data

volumes:
  postgres_data:
```

### **Passo 2: Iniciar PostgreSQL**

```bash
docker-compose up -d
```

### **Passo 3: Usar libpq UCRT64**

```bash
# Instalar apenas o cliente PostgreSQL
C:\msys64\usr\bin\bash.exe -lc "pacman -S mingw-w64-ucrt-x86_64-postgresql --noconfirm"
```

**Vantagem:** PostgreSQL roda no Docker (isolado), mas você usa as libraries UCRT64 para conectar!

---

## 🧪 **Testando a Conexão**

Após consertar, teste a conexão:

```cpp
#include "core/database/Connection.hpp"

int main() {
    try {
        Core::Database::Connection conn(
            "host=localhost port=5432 dbname=cppbackend user=cppuser password=cpppass"
        );

        auto result = conn.execute("SELECT version()");

        if (result.isSuccess()) {
            std::cout << "PostgreSQL conectado!" << std::endl;
            std::cout << "Versão: " << result.getValue(0, 0) << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Erro: " << e.what() << std::endl;
    }

    return 0;
}
```

---

## 📝 **Checklist**

- [ ] Decidir qual solução usar (1, 2 ou 3)
- [ ] Remover PostgreSQL MINGW64
- [ ] Instalar PostgreSQL UCRT64 (ou configurar Docker)
- [ ] Atualizar CMakeLists.txt com caminhos corretos
- [ ] Descomentar código do PostgreSQL no CMakeLists.txt
- [ ] Limpar e recompilar o projeto
- [ ] Testar conexão com banco

---

## 🎯 **Recomendação**

**Use Solução 3 (Docker):**

- ✅ Mais fácil de gerenciar
- ✅ Isolado do sistema
- ✅ Portável (funciona em qualquer máquina)
- ✅ Sem conflitos de compilador

**Ou use Solução 1 se quiser tudo nativo:**

- ✅ PostgreSQL UCRT64 integrado
- ✅ Menos overhead que Docker
- ⚠️ Requer cuidado com PATH e compiladores
