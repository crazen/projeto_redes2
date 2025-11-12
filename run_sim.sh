#!/bin/bash

# ===================================================================
# SCRIPT DE EXECUÇÃO - PROJETO REDES EQUIPE 6
# Itera por todos os cenários e por 33 sementes para cada um.
# ===================================================================

# --- Configurações ---
CMD="./ns3"
APP_NAME="scratch/equipe_6_2s2025" 

CLIENTES=(1 2 4 8 16 32)
TRAFEGO_TIPOS=(0 1 2) # 0=CBR, 1=Rajada, 2=Mixed
MOBILIDADE_TIPOS=(false true) # false=Estático, true=Móvel
SEMENTES=$(seq 1 5) # Executa 5 vezes

echo "================================================="
echo "Iniciando script de simulação TOTAL (Projeto Redes)"
echo "Simulador: $CMD"
echo "App: $APP_NAME"
echo "================================================="

# Loop por Clientes
for clients in "${CLIENTES[@]}"; do
    # Loop por Tipo de Tráfego
    for trafego in "${TRAFEGO_TIPOS[@]}"; do
        # Loop por Mobilidade
        for mobility in "${MOBILIDADE_TIPOS[@]}"; do
            
            # --- Define nomes amigáveis para o log ---
            T_NOME="Mixed" # Default
            [ $trafego -eq 0 ] && T_NOME="CBR"
            [ $trafego -eq 1 ] && T_NOME="Rajada"
            
            M_NOME="Movel" # Default
            [ $mobility == "false" ] && M_NOME="Estatico"
            # ------------------------------------------

            echo ""
            echo "--- Iniciando Cenário: $clients Clientes, $T_NOME, $M_NOME ---"
            
            # Loop por Sementes (Seeds)
            for seed in $SEMENTES; do
                echo "Rodando: Seed $seed de 5..."
                
                # Executa o comando de simulação
                $CMD run "$APP_NAME --nClients=$clients --trafego=$trafego --mobility=$mobility --seed=$seed"
            done
            
            echo "--- Cenário (5) Concluído ---"
        done
    done
done

echo ""
echo "================================================="
echo "TODAS AS SIMULAÇÕES FORAM CONCLUÍDAS."
echo "================================================="
