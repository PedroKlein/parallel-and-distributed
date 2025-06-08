#!/usr/bin/env python3

import csv
import os
import subprocess
import statistics
import sys
from typing import List

def calculate_stats(times: List[float]):
    """Calcula a média e o desvio padrão de uma lista de tempos."""
    if not times:
        return 0.0, 0.0
    
    mean = statistics.mean(times)
    std_dev = statistics.stdev(times) if len(times) > 1 else 0.0
    return mean, std_dev

def main():
    """
    Script para executar testes em lote da multiplicação de matrizes com MPI.
    Funciona tanto em um ambiente local quanto em um cluster gerenciado pelo SLURM.
    """
    # --- Detecção de Ambiente ---
    IS_SLURM_RUN = 'SLURM_JOB_ID' in os.environ

    # --- Validação de Argumentos ---
    if IS_SLURM_RUN:
        if len(sys.argv) != 3:
            print(f"Uso em SLURM: {sys.argv[0]} <executável> <machinefile>")
            sys.exit(1)
        executable = sys.argv[1]
        machinefile = sys.argv[2]
    else: # Execução Local
        if len(sys.argv) != 2:
            print(f"Uso Local: {sys.argv[0]} <executável>")
            sys.exit(1)
        executable = sys.argv[1]
        machinefile = None # Não usado localmente

    if not os.path.isfile(executable):
        print(f"Erro: Arquivo executável '{executable}' não foi encontrado.")
        sys.exit(1)

    # --- Configuração dos Testes ---
    job_id = os.environ.get('SLURM_JOB_ID', 'local')
    output_csv_file = f"mpi_results_{job_id}.csv"
    repetitions = 5

    comm_types = ["collective", "sync", "async_naive", "async"]
    matrix_sizes = [128, 256, 512, 1024]

    # Define o número de processos com base no ambiente
    if IS_SLURM_RUN:
        total_tasks = int(os.environ.get('SLURM_NTASKS', 1))
        print("--- Executando em ambiente SLURM ---")
    else: # Local
        total_tasks = os.cpu_count() or 4
        print("--- Executando em ambiente LOCAL ---")
        
    process_counts = [2**i for i in range(1, total_tasks.bit_length()) if 2**i <= total_tasks]
    # Garante que o número máximo de processos seja testado se não for uma potência de 2
    if not process_counts or process_counts[-1] != total_tasks:
        process_counts.append(total_tasks)

    print("Iniciando bateria de testes...")
    print(f"Executável: {executable}")
    print(f"Tipos de Comunicação: {comm_types}")
    print(f"Tamanhos das Matrizes: {matrix_sizes}")
    print(f"Número de Processos: {process_counts}")
    print(f"Repetições por teste: {repetitions}")
    print("-" * 50)

    with open(output_csv_file, mode="w", newline='') as csvfile:
        csv_writer = csv.writer(csvfile)
        csv_writer.writerow([
            "comm_type", "matrix_size", "num_procs", "environment",
            "total_time_mean", "total_time_std", "comm_time_mean", "comm_time_std",
            "comp_time_mean", "comp_time_std", "repetitions"
        ])

        for comm_type in comm_types:
            for n in matrix_sizes:
                for p in process_counts:
                    if n % p != 0:
                        continue

                    print(f"Testando: {comm_type}, n={n}, p={p}...")
                    
                    times_data = {'total': [], 'comm': [], 'comp': []}

                    for rep in range(repetitions):
                        # --- Construção do Comando Específica do Ambiente ---
                        if IS_SLURM_RUN:
                            cmd = [
                                "mpirun", "-np", str(p), "-machinefile", machinefile,
                                "--mca", "btl", "^openib", "--mca", "btl_tcp_if_include", "eno2",
                                "--bind-to", "none", executable, str(n), comm_type
                            ]
                        else: # Comando simples para execução local
                            cmd = ["mpirun", "-np", str(p), executable, str(n), comm_type]
                        
                        try:
                            result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True, check=True)
                            parts = result.stdout.strip().split(',')
                            if len(parts) == 6:
                                _, _, _, total_t, comm_t, comp_t = parts
                                times_data['total'].append(float(total_t))
                                times_data['comm'].append(float(comm_t))
                                times_data['comp'].append(float(comp_t))
                        except subprocess.CalledProcessError as e:
                            print(f"\nERRO ao executar: {' '.join(cmd)}\n  {e.stderr.strip()}")
                            break 

                    if times_data['total']:
                        total_mean, total_std = calculate_stats(times_data['total'])
                        comm_mean, comm_std = calculate_stats(times_data['comm'])
                        comp_mean, comp_std = calculate_stats(times_data['comp'])
                        
                        csv_writer.writerow([
                            comm_type, n, p, job_id,
                            f"{total_mean:.6f}", f"{total_std:.6f}",
                            f"{comm_mean:.6f}", f"{comm_std:.6f}",
                            f"{comp_mean:.6f}", f"{comp_std:.6f}",
                            repetitions
                        ])
                        print(f"  -> Resultado: Média Total = {total_mean:.6f}s")

    print("-" * 50)
    print("Bateria de testes concluída.")

if __name__ == "__main__":
    main()