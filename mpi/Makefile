# --- Variáveis Configuráveis ---
# Valores padrão que podem ser sobrescritos na linha de comando.
# Ex: make run NP=8 N=1024 COMM_TYPE=async

# Número de Processos MPI
NP ?= 4
# Tamanho da Matriz
N ?= 2048
# Tipo de Comunicação (collective, sync, async, async_naive)
COMM_TYPE ?= collective
# Comando para o interpretador Python
PYTHON = python3
# Caminho do executável gerado pelo CMake
EXECUTABLE = build/mpi_matmult


# --- Alvos (Targets) ---

# O .PHONY declara que estes alvos não produzem arquivos com o mesmo nome.
# Isso garante que eles sempre executem os comandos associados.
.PHONY: all build run validate test clean help

# O alvo 'all' é o padrão. Se você digitar apenas 'make', ele será executado.
all: build

# Compila o projeto usando o CMake.
# Depende dos arquivos fonte, headers e do CMakeLists.txt.
# Se nenhum desses arquivos mudou, a compilação não será refeita.
build: $(EXECUTABLE)

$(EXECUTABLE): src/*.c include/*.h CMakeLists.txt
	@echo "--- [BUILD] Configurando e compilando o projeto com CMake ---"
	@mkdir -p build
	@cd build && cmake .. > /dev/null
	@cd build && make
	@echo "--- [BUILD] Compilação concluída: $(EXECUTABLE) ---"

# Executa uma única instância do programa com os parâmetros definidos.
run: build
	@echo "--- [RUN] Executando com NP=$(NP), N=$(N), Tipo=$(COMM_TYPE) ---"
	@mpirun -np $(NP) $(EXECUTABLE) $(N) $(COMM_TYPE)

# Executa o programa com a flag de validação.
validate: build
	@echo "--- [VALIDATE] Executando com VALIDAÇÃO (NP=$(NP), N=$(N), Tipo=$(COMM_TYPE)) ---"
	@mpirun -np $(NP) $(EXECUTABLE) $(N) $(COMM_TYPE) --validate
	@echo "--- [VALIDATE] Validação finalizada. ---"

# Executa o script Python para a bateria de testes.
test: build
	@echo "--- [TEST] Iniciando script de testes em lote (batch tests) ---"
	@$(PYTHON) scripts/batch_test.py $(EXECUTABLE)
	@echo "--- [TEST] Script de testes finalizado. ---"

# Limpa o projeto, removendo o diretório de build e os resultados.
clean:
	@echo "--- [CLEAN] Removendo diretório de build e arquivos de resultado ---"
	@rm -rf build mpi_results.csv
	@echo "--- [CLEAN] Limpeza concluída. ---"

# Um alvo de ajuda para exibir os comandos disponíveis.
help:
	@echo "Makefile para o Projeto de Multiplicação de Matrizes com MPI"
	@echo ""
	@echo "Uso:"
	@echo "  make build          - Compila o código-fonte (padrão)."
	@echo "  make run            - Executa uma única instância do programa."
	@echo "  make validate       - Executa uma única instância com validação de resultado."
	@echo "  make test           - Executa a bateria completa de testes com o script Python."
	@echo "  make clean          - Remove todos os arquivos gerados pela compilação e testes."
	@echo ""
	@echo "É possível sobrescrever as variáveis para 'run' e 'validate':"
	@echo "  make validate NP=2 N=128 COMM_TYPE=async"
	@echo ""