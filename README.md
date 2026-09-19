# WinJalLauncher

Launcher separado para organizar containers Winlator e importar dados autorizados de jogos. Este repositório não distribui APKs de terceiros, executáveis Windows ou arquivos de jogos.

## Projetos relacionados

- WinJalBionic_Cmod: https://github.com/arthurguedesribeiro02-dev/WinJalBionic_Cmod
- Winlator Ludashi upstream: https://github.com/StevenMXZ/Winlator-Ludashi
- Winlator Cmod upstream: https://github.com/coffincolors/winlator
- Winlator Bruno Dev upstream: https://github.com/brunodev85/winlator

Os arquivos em `src/` são o ponto de partida do launcher referenciado. A integração com cada runtime deve ocorrer por importação de container e seleção de binário local autorizado.

## Fork exemplar: Winlator Ludashi

O Winlator Ludashi é usado como fork exemplar no catálogo do launcher. A detecção aceita o pacote `com.winlator.ludashi`, além de nomes visíveis e pastas que contenham `winlator` ou `ludashi`. A implementação Android de referência está em `android/UniversalLauncherActivity.java`.

O repositório não redistribui o APK do Ludashi, jogos, executáveis Windows ou runtimes de terceiros. O launcher apenas detecta instalações autorizadas e mantém cada fork isolado.

O workflow `.github/workflows/weekly-upstream-status.yml` atualiza semanalmente os metadados públicos dos upstreams e grava o resultado em `docs/UPSTREAM_STATUS.json`.
