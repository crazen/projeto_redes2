import os
import re
import glob
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

print("Iniciando script de análise de resultados...")

# --- 1. Definir o padrão dos nomes dos arquivos ---
# Este padrão de regex captura os grupos: (Tráfego), (Clientes), (Mobilidade), (Seed)
# Ele foi feito para funcionar com nomes como "resultados_equipe6_CBR-Rajada_32clientes_movel_s5.txt"
file_pattern = re.compile(
    r"resultados_run/resultados_equipe6_(.*)_(\d+)clientes_([^_]+)_s(\d+)\.txt"
)

# --- 2. Coletar dados dos arquivos ---
data = []
# Encontrar todos os arquivos .txt na pasta
files_to_parse = glob.glob("resultados_run/*.txt")

if len(files_to_parse) < 180:
    print(f"AVISO: Foram encontrados {len(files_to_parse)} arquivos, mas 180 eram esperados (6x3x2x5).")
    print("Os gráficos serão gerados, mas podem estar incompletos.")
else:
    print(f"Encontrados todos os {len(files_to_parse)} arquivos de resultado para processar...")

for f in files_to_parse:
    # Tentar extrair os parâmetros do nome do arquivo
    match = file_pattern.match(f)
    if not match:
        print(f"Aviso: Ignorando arquivo com nome inesperado: {f}")
        continue
        
    # Salvar os grupos capturados
    trafego, n_clients, mobilidade, seed = match.groups()
    
    # ✅ CORREÇÃO: Renomeia "CBR-Rajada" para "MIXED"
    trafego_nome = trafego.replace("CBR-Rajada", "MIXED (CBR+Rajada)")
    
    # Dicionário para guardar as métricas deste arquivo
    metrics = {
        "Tráfego": trafego_nome,
        "Clientes": int(n_clients),
        "Mobilidade": "Móvel" if mobilidade == "movel" else "Estático",
        "Seed": int(seed)
    }
    
    # --- 3. Ler o conteúdo do arquivo para extrair as métricas ---
    with open(f, 'r') as file_content:
        for line in file_content:
            if line.startswith("Vazao_Mbps:"):
                metrics["Vazão (Mbps)"] = float(line.split(":")[1].strip())
            elif line.startswith("Atraso_ms:"):
                metrics["Atraso (ms)"] = float(line.split(":")[1].strip())
            elif line.startswith("Perda_%:"):
                metrics["Perda (%)"] = float(line.split(":")[1].strip())
                
    data.append(metrics)

print(f"Processados {len(data)} arquivos com sucesso.")

# --- 4. Criar DataFrame com Pandas e Calcular Médias ---
if not data:
    print("ERRO: Nenhum dado foi processado. Verifique os nomes dos arquivos.")
    exit()

df = pd.DataFrame(data)

# Calcular a média de todas as sementes
df_avg = df.groupby(['Clientes', 'Tráfego', 'Mobilidade']).mean().reset_index()

# Remover a coluna 'Seed' da média, pois não faz sentido
if 'Seed' in df_avg.columns:
    df_avg = df_avg.drop(columns='Seed')

print("\n--- Médias dos Resultados (agrupadas por 5 sementes) ---")
print(df_avg.to_string())

# --- 5. Gerar Gráficos com Seaborn (Estilo melhorado) ---
print("\nGerando gráficos...")
# ✅ GRÁFICOS MAIORES E MAIS NÍTIDOS:
# Usando 'darkgrid' para melhor contraste e 'poster' para fontes maiores
sns.set_theme(style="darkgrid", context="poster", font_scale=0.8) 
plt.rcParams['figure.figsize'] = (16, 10) # Tamanho da figura (largura, altura)
plt.rcParams['figure.dpi'] = 100 # Resolução

# Nomes dos gráficos
plot_files = []
# Ordenar os tipos de tráfego para uma legenda lógica
hue_order = ["CBR", "MIXED (CBR+Rajada)", "Rajada"]

# --- Gráfico 1: VAZÃO ---
plt.figure()
ax = sns.lineplot(
    data=df_avg, 
    x="Clientes", 
    y="Vazão (Mbps)", 
    hue="Tráfego",       
    hue_order=hue_order, # Ordem da legenda
    style="Mobilidade",  
    markers=True,        
    markersize=14,       # Marcadores maiores
    linewidth=3.5        # Linhas mais grossas
)
ax.set_title("Vazão Agregada vs. Número de Clientes", fontsize=24, pad=20)
ax.set_xlabel("Número de Clientes", fontsize=18)
ax.set_ylabel("Vazão Agregada (Mbps)", fontsize=18)
ax.set_xticks(df_avg["Clientes"].unique()) 
plt.legend(title="Cenário", loc='upper left', bbox_to_anchor=(1, 1))
plt.tight_layout(rect=[0, 0, 0.82, 1]) # Ajusta para a legenda caber
file_vazao = "grafico_vazao_agregada.png"
plt.savefig(file_vazao)
plot_files.append(file_vazao)
plt.close()

# --- Gráfico 2: PERDA DE PACOTES ---
plt.figure()
ax = sns.lineplot(
    data=df_avg, 
    x="Clientes", 
    y="Perda (%)", 
    hue="Tráfego",
    hue_order=hue_order,
    style="Mobilidade",
    markers=True,
    markersize=14,
    linewidth=3.5
)
ax.set_title("Perda de Pacotes vs. Número de Clientes", fontsize=24, pad=20)
ax.set_xlabel("Número de Clientes", fontsize=18)
ax.set_ylabel("Perda de Pacotes (%)", fontsize=18)
ax.set_xticks(df_avg["Clientes"].unique())
plt.legend(title="Cenário", loc='upper left', bbox_to_anchor=(1, 1))
plt.tight_layout(rect=[0, 0, 0.82, 1])
file_perda = "grafico_perda_pacotes.png"
plt.savefig(file_perda)
plot_files.append(file_perda)
plt.close()

# --- Gráfico 3: ATRASO MÉDIO ---
plt.figure()
ax = sns.lineplot(
    data=df_avg, 
    x="Clientes", 
    y="Atraso (ms)", 
    hue="Tráfego",
    hue_order=hue_order,
    style="Mobilidade",
    markers=True,
    markersize=14,
    linewidth=3.5
)
ax.set_title("Atraso Médio vs. Número de Clientes", fontsize=24, pad=20)
ax.set_xlabel("Número de Clientes", fontsize=18)
ax.set_ylabel("Atraso Médio (ms)", fontsize=18)
ax.set_xticks(df_avg["Clientes"].unique())
plt.legend(title="Cenário", loc='upper left', bbox_to_anchor=(1, 1))
plt.tight_layout(rect=[0, 0, 0.82, 1])
file_atraso = "grafico_atraso_medio.png"
plt.savefig(file_atraso)
plot_files.append(file_atraso)
plt.close()

print("\n--- Concluído! ---")
print("Gráficos salvos em:")
for f in plot_files:
    print(f"✅ {f}")
print("\nTabela de médias dos dados:")
print(df_avg.to_string())
