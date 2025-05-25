import subprocess
import csv
import os
from datetime import datetime

executables = {
    "coletiva": "./mpi_coletiva",
    "p2p_bloq": "./mpi_p2p_bloqueante",
    "p2p_nbloq": "./mpi_p2p_naobloqueante"
}

matrix_sizes = [256, 512, 1024]  # ou maiores, conforme memória
process_counts = [2, 4, 8]

output_file = "resultados_tempo.csv"

with open(output_file, mode="w", newline="") as file:
    writer = csv.writer(file)
    writer.writerow(["implementacao", "tamanho_matriz", "num_processos", "tempo_execucao"])

    for impl, exec_path in executables.items():
        for n in matrix_sizes:
            for p in process_counts:
                cmd = ["mpirun", "-np", str(p), exec_path, str(n)]
                print(f"Executando: {cmd}")
                try:
                    result = subprocess.run(cmd, capture_output=True, text=True, timeout=300)
                    output = result.stdout
                    # Extrai tempo do output
                    for line in output.splitlines():
                        if "Execution time" in line:
                            tempo = float(line.split(":")[1])
                            writer.writerow([impl, n, p, tempo])
                            break
                except subprocess.TimeoutExpired:
                    print(f"Timeout em {cmd}")
                    writer.writerow([impl, n, p, "TIMEOUT"])
                except Exception as e:
                    print(f"Erro ao executar {cmd}: {e}")
                    writer.writerow([impl, n, p, "ERRO"])
