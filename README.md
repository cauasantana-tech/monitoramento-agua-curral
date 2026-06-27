# 💧 Sistema de Monitoramento de Qualidade da Água (Curral)

Projeto final desenvolvido para a disciplina de Internet das Coisas (IoT). O sistema visa automatizar a leitura e o monitoramento da qualidade da água consumida pelo gado em currais, garantindo parâmetros saudáveis e alertando produtores sobre a necessidade de manutenção.

## 🏗️ Arquitetura do Sistema

O projeto foi construído seguindo as 4 camadas clássicas de IoT:

- **Camada de Percepção (Sensoriamento e Hardware):** Utiliza um microcontrolador ESP32 equipado com Wi-Fi nativo. A leitura do ambiente é feita por um sensor de Nível (Ultrassônico) e um sensor de Temperatura (DHT22). Sensores químicos (pH, Turbidez e Condutividade) foram simulados via software para validação da rede.
- **Camada de Rede (Conectividade):** O ESP32 conecta-se ao Wi-Fi local para despachar pacotes de dados via protocolo HTTP.
- **Camada de Serviço (Nuvem e Dados):** Os dados são enviados para um banco de dados relacional (Supabase/PostgreSQL) via REST API, garantindo o armazenamento cronológico do histórico de aferição de cada sensor.
- **Camada de Aplicação (Interface e Dashboard):** Painel web desenvolvido em HTML/JS com *Chart.js*, exibindo gráficos em tempo real de cada unidade de medida. O sistema conta com um Sistema de Alertas que gera notificações em caso de contaminação, além de fornecer indicações de estado normal e recomendações automáticas (ex: "Necessário limpeza do reservatório").

## 🚀 Como visualizar

O dashboard interativo em tempo real está hospedado via GitHub Pages. 
Acesse o link do ambiente de produção nas configurações deste repositório para visualizar os dados sendo transmitidos pelo ESP32.