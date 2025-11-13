/* ============================================================================
 * PROJETO REDES 2S2025 - UNIFESP/ICT - EQUIPE 6
 * VERSÃO FINAL (v5) - GARGALO DE WIFI REALISTA
 * - Remove gargalo artificial de 2Mbps (volta para 100Mbps).
 * - Usa LogDistancePropagationLossModel para criar um gargalo de WiFi
 * realista, que é afetado pela mobilidade.
 * ============================================================================ */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/propagation-loss-model.h"
#include <fstream>
#include <iomanip>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ProjetoRedesEquipe6");

int main(int argc, char *argv[])
{
    // ===========================
    //  CONFIGURAÇÕES GLOBAIS
    // ===========================
    Time::SetResolution(Time::NS);
    
    // Configurações TCP conforme documentação
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1448)); 
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", 
                       StringValue("ns3::TcpNewReno"));

    // ===========================
    //  PARÂMETROS CONFORME TABELA 1
    // ===========================
    uint32_t nClients = 1;
    uint32_t trafego = 0;  // 0=CBR, 1=Rajada, 2=CBR/Rajada
    bool mobilityOn = false;
    uint32_t seed = 1;

    CommandLine cmd(__FILE__);
    cmd.AddValue("nClients", "Número de clientes WiFi", nClients);
    cmd.AddValue("trafego", "Tipo de tráfego (0=CBR, 1=Rajada, 2=CBR/Rajada)", trafego);
    cmd.AddValue("mobility", "Habilitar mobilidade", mobilityOn);
    cmd.AddValue("seed", "Semente para aleatoriedade", seed);
    cmd.Parse(argc, argv);

    // Configurar semente para melhorar resultados
    SeedManager::SetSeed(seed);
    SeedManager::SetRun(seed);

    // ===========================
    //  CONFIGURAÇÕES DE TEMPO
    // ===========================
    double totalTime = 60.0;  
    double startTime = 1.0;   
    double stopTime = totalTime; // Parada das aplicações
    
    // Dimensões do cenário
    double areaSize = 140.0;

    std::cout << "\n==========================================\n";
    std::cout << "  PROJETO REDES 2S2025 - UNIFESP/ICT\n";
    std::cout << "  EQUIPE 6 - COMPORTAMENTO ESPERADO\n";
    std::cout << "==========================================\n";
    std::cout << "Clientes: " << nClients << " | ";
    
    // Nome do tráfego
    std::string trafegoNome;
    if (trafego == 0) trafegoNome = "CBR";
    else if (trafego == 1) trafegoNome = "Rajada";
    else trafegoNome = "CBR-Rajada";
    
    std::cout << "Tráfego: " << trafegoNome 
              << " | Mobilidade: " << (mobilityOn ? "Sim" : "Não") << "\n";
    std::cout << "Cenário: " << areaSize << "x" << areaSize << "m | WiFi: 802.11a\n";
    std::cout << "Tempo: " << totalTime << "s | Semente: " << seed << "\n";
    std::cout << "==========================================\n";

    // ===========================
    //  TOPOLOGIA CONFORME FIGURA 1
    // ===========================
    NodeContainer serverNode;     // s2
    serverNode.Create(1);
    
    NodeContainer routerNodes;    // s1 e s0
    routerNodes.Create(2);
    
    NodeContainer apNode;         // AP
    apNode.Create(1);
    
    NodeContainer wifiStaNodes;   // c0, c1, ..., cn
    wifiStaNodes.Create(nClients);

    // ===========================
    //  REDE CABEADA - 100Mbps
    // ===========================
    PointToPointHelper p2p;
    
    // Enlace: 100Mbps.
    p2p.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
    
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));

    // Conexões: s2-s1-s0-AP conforme figura
    NetDeviceContainer devices_s2s1 = p2p.Install(serverNode.Get(0), routerNodes.Get(0));
    NetDeviceContainer devices_s1s0 = p2p.Install(routerNodes.Get(0), routerNodes.Get(1));
    NetDeviceContainer devices_s0ap = p2p.Install(routerNodes.Get(1), apNode.Get(0));

    // ===========================
    //  REDE WI-FI 802.11a - CONFORME DOCUMENTAÇÃO
    // ===========================
    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211a);
    
    wifi.SetRemoteStationManager("ns3::AarfWifiManager");

    WifiMacHelper wifiMac;
    YansWifiChannelHelper wifiChannel;
    
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss("ns3::LogDistancePropagationLossModel",
                                   "Exponent", DoubleValue(2.5),
                                   "ReferenceDistance", DoubleValue(1.0),
                                   "ReferenceLoss", DoubleValue(46.6777)); // Valor padrão, bom

    YansWifiPhyHelper wifiPhy;
    wifiPhy.SetChannel(wifiChannel.Create());
    
    // 16 dBm - CONFORME TABELA 1
    wifiPhy.Set("TxPowerStart", DoubleValue(16.0));
    wifiPhy.Set("TxPowerEnd", DoubleValue(16.0));

    // SSID conforme especificação
    Ssid ssid = Ssid("Equipe6");

    // Dispositivos cliente
    wifiMac.SetType("ns3::StaWifiMac",
                    "Ssid", SsidValue(ssid),
                    "ActiveProbing", BooleanValue(false));
    NetDeviceContainer staDevices = wifi.Install(wifiPhy, wifiMac, wifiStaNodes);

    // Access Point
    wifiMac.SetType("ns3::ApWifiMac",
                    "Ssid", SsidValue(ssid));
    NetDeviceContainer apDevices = wifi.Install(wifiPhy, wifiMac, apNode);

    // ===========================
    //  MOBILIDADE - CONFORME DOCUMENTAÇÃO
    // ===========================
    MobilityHelper mobility;

    // AP no centro do cenário
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
    positionAlloc->Add(Vector(areaSize/2, areaSize/2, 0.0)); // Centro: (70,70)
    mobility.SetPositionAllocator(positionAlloc);
    mobility.Install(apNode);

    MobilityHelper staMobility;
    //Print para debug
    if (mobilityOn) {
        // CENÁRIO COM MOBILIDADE: ConstantVelocity 1-2 m/s (3.6-7.2 km/h)
        staMobility.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
        std::cout << "Mobilidade: ConstantVelocity (1-2 m/s = 3.6-7.2 km/h)\n";
    } else {
        // CENÁRIO SEM MOBILIDADE: ConstantPosition
        staMobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
        std::cout << "Mobilidade: ConstantPosition (0 km/h)\n";
    }

    // Posicionamento inicial conforme descrição: organizados em grid
    staMobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                     "MinX", DoubleValue(areaSize/2),
                                     "MinY", DoubleValue(areaSize/2),
                                     "DeltaX", DoubleValue(5.0),
                                     "DeltaY", DoubleValue(5.0),
                                     "GridWidth", UintegerValue(3),
                                     "LayoutType", StringValue("RowFirst"));
    staMobility.Install(wifiStaNodes);

    // Aplicar velocidade aleatória se mobilidade estiver ativa
    if (mobilityOn) {
        // Criar geradores de variáveis aleatórias
        Ptr<UniformRandomVariable> randSpeed = CreateObject<UniformRandomVariable>();
        randSpeed->SetAttribute("Min", DoubleValue(1.0)); // 1.0 m/s
        randSpeed->SetAttribute("Max", DoubleValue(2.0)); // 2.0 m/s

        Ptr<UniformRandomVariable> randAngle = CreateObject<UniformRandomVariable>();
        randAngle->SetAttribute("Min", DoubleValue(0.0));
        randAngle->SetAttribute("Max", DoubleValue(2 * M_PI)); // 0 a 360 graus

        for (uint32_t i = 0; i < wifiStaNodes.GetN(); ++i) {
            Ptr<ConstantVelocityMobilityModel> mob = 
                wifiStaNodes.Get(i)->GetObject<ConstantVelocityMobilityModel>();
            
            // Sortear/randomize velocidade e direção para CADA cliente
            double speed = randSpeed->GetValue();
            double angle = randAngle->GetValue();
            
            mob->SetVelocity(Vector(speed * cos(angle), speed * sin(angle), 0));
        }
    }

    // ===========================
    //  PILHA TCP/IP
    // ===========================
    InternetStackHelper stack;
    stack.Install(serverNode);
    stack.Install(routerNodes);
    stack.Install(apNode);
    stack.Install(wifiStaNodes);

    // Endereçamento IP conforme topologia
    Ipv4AddressHelper address;

    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces_s2s1 = address.Assign(devices_s2s1);

    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces_s1s0 = address.Assign(devices_s1s0);

    address.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces_s0ap = address.Assign(devices_s0ap);

    address.SetBase("192.168.0.0", "255.255.255.0");
    /*Ipv4InterfaceContainer apInterface =*/ address.Assign(apDevices);
    Ipv4InterfaceContainer staInterfaces = address.Assign(staDevices);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // ===========================
    //  APLICAÇÕES - CONFORME ESPECIFICAÇÃO
    // ===========================
    ApplicationContainer serverApps;
    ApplicationContainer clientApps;

    uint16_t port = 9;
    Ipv4Address serverAddress = interfaces_s2s1.GetAddress(0);

    std::cout << "\nConfiguração de Aplicações:\n";

    // Distribuição de clientes
    for (uint32_t i = 0; i < nClients; ++i) {
        Ipv4Address clientAddress = staInterfaces.GetAddress(i);
        
        // Determinar tipo de tráfego para este cliente
        bool isUdp = false;
        if (trafego == 0) {
            // Todos CBR (UDP)
            isUdp = true;
        } else if (trafego == 1) {
            // Todos Rajada (TCP)
            isUdp = false;
        } else if (trafego == 2) {
            // 50% CBR, 50% Rajada
            isUdp = (i % 2 == 0); // id par = CBR, id ímpar = Rajada
        }

        if (isUdp) {
            // TRÁFEGO CBR - UDP: 512 kbps, 512 bytes
            // Servidor (transmissor)
            OnOffHelper onoff("ns3::UdpSocketFactory", 
                            InetSocketAddress(clientAddress, port));
            onoff.SetConstantRate(DataRate("512kbps"), 512); // 512 bytes
            onoff.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
            onoff.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
            serverApps.Add(onoff.Install(serverNode.Get(0)));

            // Cliente (receptor)
            PacketSinkHelper sink("ns3::UdpSocketFactory",
                                InetSocketAddress(Ipv4Address::GetAny(), port));
            clientApps.Add(sink.Install(wifiStaNodes.Get(i)));

            std::cout << "Cliente " << i << " (" << clientAddress << "): CBR (UDP) - 512kbps, 512 bytes\n";
        } else {
            // TRÁFEGO RAJADA - TCP: tamanho máximo, 1500 bytes
            // Servidor (transmissor)
            BulkSendHelper bulk("ns3::TcpSocketFactory",
                                InetSocketAddress(clientAddress, port));
            bulk.SetAttribute("MaxBytes", UintegerValue(0)); // Transmissão contínua
            bulk.SetAttribute("SendSize", UintegerValue(1500)); // 1500 bytes
            serverApps.Add(bulk.Install(serverNode.Get(0)));

            // Cliente (receptor)
            PacketSinkHelper sink("ns3::TcpSocketFactory",
                                InetSocketAddress(Ipv4Address::GetAny(), port));
            clientApps.Add(sink.Install(wifiStaNodes.Get(i)));

            std::cout << "Cliente " << i << " (" << clientAddress << "): Rajada (TCP) - 1500 bytes\n";
        }
        
        port++; // Porta diferente para cada cliente
    }

    // Configurar tempos das aplicações
    serverApps.Start(Seconds(startTime));
    serverApps.Stop(Seconds(stopTime));
    clientApps.Start(Seconds(0.0));
    clientApps.Stop(Seconds(stopTime + 1));

    // ===========================
    //  SIMULAÇÃO E MONITORAMENTO
    // ===========================
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();

    std::cout << "\n>>> Iniciando simulação...\n";
    Simulator::Stop(Seconds(stopTime + 2));
    Simulator::Run();

    // ===========================
    //  COLETA DE RESULTADOS
    // ===========================
    monitor->CheckForLostPackets();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowmon.GetClassifier());
    FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats();

    // Métricas agregadas
    double totalDelay = 0.0;
    uint64_t totalTxPackets = 0;
    uint64_t totalRxPackets = 0;
    uint64_t totalLostPackets = 0;
    uint64_t totalRxBytes = 0; 
    uint32_t flowCountRxPackets = 0; 

    std::cout << "\n=== ANÁLISE DETALHADA POR FLUXO ===\n";

    for (auto iter = stats.begin(); iter != stats.end(); ++iter) {
        FlowId flowId = iter->first;
        FlowMonitor::FlowStats flowStats = iter->second;
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(flowId);

        // Considerar apenas fluxos do servidor para os clientes
        if (t.sourceAddress == serverAddress) {
            
            // Garantir que não haja divisão por zero se rxPackets == 0
            double delay = 0.0;
            if (flowStats.rxPackets > 0) {
                delay = (flowStats.delaySum.GetSeconds() / flowStats.rxPackets) * 1000.0; // ms
            }

            // Tempo de medição correto
            double duration = stopTime - startTime;
            // Tenta pegar uma duração mais precisa do fluxo
            if (flowStats.timeLastRxPacket.GetSeconds() > flowStats.timeFirstTxPacket.GetSeconds()) {
                 duration = flowStats.timeLastRxPacket.GetSeconds() - flowStats.timeFirstTxPacket.GetSeconds();
            }
            if (duration <= 0) duration = stopTime - startTime; // Fallback

            double throughput = flowStats.rxBytes * 8.0 / duration / 1000000.0; // Mbps
            
            double lossPercent = (flowStats.txPackets > 0) ? 
                (100.0 * flowStats.lostPackets / flowStats.txPackets) : 0.0;

            std::string protocol = (t.protocol == 6) ? "TCP" : "UDP";
            
            std::cout << "Fluxo " << flowId << " [" << protocol << "] "
                      << t.sourceAddress << " -> " << t.destinationAddress
                      << " | Vazão: " << std::fixed << std::setprecision(3) << throughput << " Mbps"
                      << " | Atraso: " << std::setprecision(2) << delay << " ms"
                      << " | Perda: " << std::setprecision(2) << lossPercent << "%"
                      << " | TX: " << flowStats.txPackets 
                      << " RX: " << flowStats.rxPackets 
                      << " Lost: " << flowStats.lostPackets << "\n";

            // Somar para as médias globais
            if (flowStats.rxPackets > 0) {
                 totalDelay += flowStats.delaySum.GetSeconds();
                 flowCountRxPackets += flowStats.rxPackets; // Ponderar pelo número de pacotes
            }
            totalRxBytes += flowStats.rxBytes; 
            totalTxPackets += flowStats.txPackets;
            totalRxPackets += flowStats.rxPackets;
            totalLostPackets += flowStats.lostPackets;
        }
    }
    
    // Cálculo das médias
    double avgThroughput = (totalRxBytes * 8.0) / (stopTime - startTime) / 1000000.0; // Vazão
    double avgDelay = (flowCountRxPackets > 0) ? (totalDelay / flowCountRxPackets) * 1000.0 : 0.0; // Atraso médio por pacote
    double avgLossPercent = (totalTxPackets > 0) ? 
        (100.0 * totalLostPackets / totalTxPackets) : 0.0;
    double pdr = (totalTxPackets > 0) ? 
        (100.0 * totalRxPackets / totalTxPackets) : 0.0;


    // ===========================
    //  RELATÓRIO FINAL
    // ===========================
    std::cout << "\n==========================================\n";
    std::cout << "RESULTADOS FINAIS - EQUIPE 6\n";
    std::cout << "------------------------------------------\n";
    std::cout << "Configuração: " << nClients << " clientes, " << trafegoNome 
              << ", Mobilidade: " << (mobilityOn ? "Sim" : "Não") << "\n";
    std::cout << "Pacotes: TX=" << totalTxPackets << " | RX=" << totalRxPackets 
              << " | Lost=" << totalLostPackets << "\n";
    std::cout << "Vazão Agregada: " << std::fixed << std::setprecision(3) << avgThroughput << " Mbps\n";
    std::cout << "Atraso Médio: " << std::setprecision(2) << avgDelay << " ms\n";
    std::cout << "Perda de Pacotes: " << std::setprecision(2) << avgLossPercent << "%\n";
    std::cout << "PDR (Packet Delivery Ratio): " << std::setprecision(2) << pdr << "%\n";
    std::cout << "==========================================\n";

    // ===========================
    //  SALVAMENTO EM ARQUIVO
    // ===========================
    std::ostringstream filename;
    // Salva na pasta "resultados_run"
    filename << "resultados_run/resultados_equipe6_" << trafegoNome << "_" << nClients
             << "clientes_" << (mobilityOn ? "movel" : "estatico") << "_s" << seed << ".txt";

    std::ofstream outFile(filename.str());
    outFile << "PROJETO REDES 2S2025 - UNIFESP/ICT - EQUIPE 6\n";
    outFile << "RESULTADOS DE SIMULAÇÃO\n";
    outFile << "Clientes: " << nClients << "\n";
    outFile << "Tráfego: " << trafegoNome << "\n";
    outFile << "Mobilidade: " << (mobilityOn ? "Sim" : "Não") << "\n";
    outFile << "Semente: " << seed << "\n";
    outFile << "Vazao_Mbps: " << std::fixed << std::setprecision(3) << avgThroughput << "\n";
    outFile << "Atraso_ms: " << std::setprecision(2) << avgDelay << "\n";
    outFile << "Perda_%: " << std::setprecision(2) << avgLossPercent << "\n";
    outFile << "PDR_%: " << std::setprecision(2) << pdr << "\n";
    outFile << "Pacotes_TX: " << totalTxPackets << "\n";
    outFile << "Pacotes_RX: " << totalRxPackets << "\n";
    outFile << "Pacotes_Perdidos: " << totalLostPackets << "\n";
    outFile.close();

    std::cout << "\n Resultados salvos em: " << filename.str() << "\n\n";

    Simulator::Destroy();
    return 0;
}
