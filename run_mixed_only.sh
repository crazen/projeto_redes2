#!/bin/bash

# ===================================================================
# SCRIPT DE EXECUÇÃO - APENAS CENÁRIOS "MIXED" (trafego=2)
# ===================================================================

CMD="./ns3"
APP_NAME="scratch/equipe_6_2s2025"
CLIENTES=(1 2 4 8 16 32)
MOBILIDADE_TIPOS=(false true)
SEMENTES=$(seq 1 5) # Usando 5 sementes

echo "================================================="
echo "Rodando APENAS os 60 testes MIXED (trafego=2)..."
echo "================================================="

for clients in "${CLIENTES[@]}"; do
    for mobility in "${MOBILIDADE_TIPOS[@]}"; do
        
        M_NOME="Movel"
        [ $mobility == "false" ] && M_NOME="Estatico"
        
        echo ""
        echo "--- Iniciando Cenário: $clients Clientes, MIXED, $M_NOME ---"
        
        for seed in $SEMENTES; do
            echo "Rodando: Seed $seed de 5..."
            # Executa o comando apenas para --trafego=2
            $CMD run "$APP_NAME --nClients=$clients --trafego=2 --mobility=$mobility --seed=$seed"
        done
        
        echo "--- Cenário (5 sementes) Concluído ---"
    done
done

echo ""
echo "================================================="
echo "TODAS AS SIMULAÇÕES MIXED FORAM CONCLUÍDAS."
echo "================================================="
