Projeto de Simulação de Redes 2S2025 - UNIFESP/ICT - Equipe 6
Este repositório contém os artefatos de simulação para o projeto prático da UC 2617 - Redes de Computadores, da Universidade Federal de São Paulo (UNIFESP).

O objetivo deste projeto é analisar o desempenho de uma rede sem fio padrão IEEE 802.11a em modo infraestruturado, utilizando o simulador Network Simulator 3 (NS-3). A análise foca em como a variação no número de clientes, a presença de mobilidade e os diferentes protocolos de transporte (TCP vs. UDP) impactam as principais métricas de rede: Vazão, Atraso e Perda de Pacotes.

Sumário
Cenários Implementados

Tecnologias Utilizadas

Estrutura do Repositório

Como Executar a Simulação

Metodologia e Parâmetros de Simulação

Análise dos Resultados

Autores

🌎 Cenários Implementados
Este projeto executa uma matriz de simulações para comparar os seguintes cenários:

Protocolos (Tráfego):

CBR (UDP): Tráfego de taxa de bits constante, não confiável.

Rajada (TCP): Tráfego "guloso" que tenta usar o máximo de banda, com controle de congestionamento.


MIXED (CBR/Rajada): A rede é dividida, com 50% dos clientes usando UDP e 50% usando TCP .

Número de Clientes:

1, 2, 4, 8, 16 e 32 clientes.

Mobilidade:

Estático: Os clientes permanecem parados em um grid próximo ao AP.


Móvel: Os clientes se movem aleatoriamente com velocidade de 1 a 2 m/s.

🛠️ Tecnologias Utilizadas
Simulador: Network Simulator 3 (ns-3), versão 3.43

Linguagem da Simulação: C++

Automação: Shell Script (.sh)

Análise de Dados: Python 3

Bibliotecas Python: pandas, matplotlib, seaborn

📁 Estrutura do Repositório
.
├── scratch/
│   └── equipe_6_2s2025.cc   # O código-fonte principal da simulação NS-3
├── resultados_run/
│   ├── ... (180 arquivos .txt com os resultados brutos) ...
├── run_all_sims.sh          # Script de shell para rodar TODAS as simulações
├── run_mixed_only.sh        # Script auxiliar (apenas para debug)
├── gerar_graficos.py        # Script Python para analisar os .txt e gerar gráficos
├── grafico_vazao_agregada.png  # Gráfico de saída
├── grafico_perda_pacotes.png   # Gráfico de saída
├── grafico_atraso_medio.png    # Gráfico de saída
└── README.md                # Este arquivo
🚀 Como Executar a Simulação
Siga estes passos para replicar os resultados do zero.

1. Pré-requisitos
Ambiente NS-3: O código foi desenvolvido e testado na versão ns-3.43 em um ambiente Linux (WSL). É necessário ter o ns-3 compilado.

Dependências Python: Instale as bibliotecas de análise de dados.

Bash

# O sistema pode exigir o uso do apt (Debian/Ubuntu)
sudo apt install python3-pandas python3-matplotlib python3-seaborn

# Ou, se preferir o pip (em um ambiente virtual ou global):
pip install pandas matplotlib seaborn
Posicionamento dos Arquivos:

Coloque o arquivo equipe_6_2s2025.cc dentro da pasta scratch/ do seu ns-3.

Coloque os scripts run_all_sims.sh e gerar_graficos.py na pasta raiz do ns-3 (ex: ns-3.43/).

2. Fluxo de Execução
O processo é dividido em duas etapas: rodar as simulações (C++) e depois analisar os resultados (Python).

Etapa 1: Rodar as Simulações
Crie a pasta de resultados:

Bash

mkdir resultados_run
Dê permissão de execução ao script:

Bash

chmod +x run_all_sims.sh
Execute as simulações:

Bash

./run_all_sims.sh
Atenção: Este script executará um total de 180 simulações (6 clientes * 3 tráfegos * 2 mobilidades * 5 sementes). Em hardware com pouca RAM (ex: 8GB), isso pode levar mais de 15 horas. Para um teste rápido, edite o arquivo run_all_sims.sh e mude a linha SEMENTES=$(seq 1 5) para SEMENTES=$(seq 1 2).

Etapa 2: Gerar os Gráficos
Após a conclusão do script run_all_sims.sh, a pasta resultados_run/ estará cheia com 180 arquivos .txt.

Execute o script de análise:

Bash

python3 gerar_graficos.py
Pronto! O script irá ler todos os 180 arquivos, calcular as médias de cada cenário e salvar os três gráficos finais (grafico_vazao_agregada.png, grafico_perda_pacotes.png e grafico_atraso_medio.png) na pasta principal.

🔬 Metodologia e Parâmetros de Simulação
Esta simulação foi configurada para ser o mais fiel possível aos requisitos do projeto e ao relatório de exemplo  fornecido.

Topologia
A rede consiste em um serverNode (s2) conectado via PointToPointHelper a dois roteadores (s1, s0) e, finalmente, ao apNode (Access Point) . Os clientes (wifiStaNodes) se conectam ao apNode via WiFi.

Parâmetros de Rede

Enlace Cabeado (P2P): 100 Mbps, 2ms de atraso.


WiFi: IEEE 802.11a.


Potência de Transmissão: 16 dBm.


Controle de Taxa: AarfWifiManager. Este é o "cérebro" do WiFi, que ajusta a velocidade de transmissão com base na qualidade do sinal.


Posicionamento (Estático): Os clientes são posicionados em um grid 3xN, com 5m de espaçamento, iniciando no centro do cenário (70, 70).

A Decisão Crítica: LogDistance vs. Friis
Havia uma contradição nos documentos do projeto:

O texto do relatório de exemplo menciona o uso de FriisPropagationLossModel.

Os gráficos do relatório de exemplo mostram que a performance "Móvel" é significativamente pior que a "Estática" .

O modelo Friis é muito otimista (simula um sinal perfeito) e, em nossos testes, os resultados "Móvel" e "Estático" eram idênticos. Isso ocorre porque o sinal permanecia perfeito mesmo com o movimento.

Para replicar o comportamento dos gráficos (onde a mobilidade degrada o sinal), esta simulação utiliza o LogDistancePropagationLossModel. Este modelo realista cria um gargalo natural no WiFi (medido em nossos testes de TCP em ~17-25 Mbps) e, crucialmente, faz com que o sinal piore com a distância, permitindo ao AarfWifiManager reduzir a taxa e mostrar o impacto real da mobilidade.

Parâmetros de Tráfego

CBR (UDP): Taxa constante de 512 kbps, com pacotes de 512 bytes.


Rajada (TCP): Tráfego BulkSend (máxima vazão possível), com pacotes de 1500 bytes.


Sementes (Seeds): O relatório pedia 33 sementes. Nossos testes foram feitos com 5 sementes por cenário (total de 180 execuções) devido a limitações de hardware. A seed garante que cada execução "Móvel" tenha um caminho aleatório diferente, permitindo o cálculo de uma média estatisticamente válida.

📊 Análise dos Resultados
Os gráficos gerados pelo script gerar_graficos.py contam uma história clara sobre o desempenho da rede.

1. Gráfico de Vazão Agregada
Este gráfico mostra a capacidade total da rede.

Rajada (TCP): A linha verde (Estático) e a laranja (Móvel) mostram o TCP se comportando perfeitamente. Ele sobe rápido e satura (fica plano) na capacidade máxima do WiFi (~25 Mbps para Estático, ~23 Mbps para Móvel). O TCP é "educado" e se auto-regula.

CBR (UDP): A linha azul sobe em linha reta perfeita (Vazão = N * 0.54 Mbps). O UDP é "mal-educado" e não se importa com a capacidade da rede.

MIXED: A linha laranja (MIXED) fica no meio. Os clientes TCP preenchem a banda que os clientes UDP "deixam" sobrar.

2. Gráfico de Perda de Pacotes

Rajada (TCP): As linhas verde e laranja (Estático e Móvel) ficam sempre próximas de 0%. O TCP detecta o congestionamento e desacelera para evitar a perda.

CBR (UDP): A linha azul mostra o colapso da rede.

De 1 a 16 clientes, a demanda (máx. 8.6 Mbps) é menor que a capacidade do WiFi (~17-25 Mbps), então a perda é 0%.

Em 32 clientes, a demanda (17.28 Mbps) ultrapassa a capacidade.

No cenário Estático (linha sólida azul), a perda começa a aparecer (2.2%).

No cenário Móvel (linha tracejada azul), a capacidade da rede cai (devido ao sinal ruim) e o UDP, por não desacelerar, causa um colapso catastrófico: a perda de pacotes dispara para ~29%.

3. Gráfico de Atraso Médio
Este gráfico mostra o tempo de espera na fila.

Rajada (TCP) e MIXED: As curvas verde e laranja sobem constantemente. Isso é esperado (e conhecido como bufferbloat). O TCP intencionalmente mantém a fila do AP cheia para saber que encontrou a capacidade máxima, o que aumenta o atraso.

CBR (UDP): A curva azul (Estático) tem um atraso baixo e perfeito até 16 clientes (rede vazia). Em 32 clientes, quando a rede congestiona, o atraso explode de 8ms para 172ms. Isso mostra a fila do AP enchendo instantaneamente, pouco antes de começar a descartar os pacotes. No cenário Móvel, o atraso é ainda maior devido à instabilidade do canal.
