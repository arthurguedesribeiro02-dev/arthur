# WinJalLauncher

Launcher universal para organizar containers Winlator e importar dados autorizados de jogos. O site público é <https://gtavforks-abncikdo.manus.space>. Este repositório não distribui APKs de terceiros, executáveis Windows ou arquivos de jogos.

## Projetos relacionados

- WinJalBionic_Cmod: https://github.com/arthurguedesribeiro02-dev/WinJalBionic_Cmod
- Winlator Ludashi upstream: https://github.com/StevenMXZ/Winlator-Ludashi
- Winlator Cmod upstream: https://github.com/coffincolors/winlator
- Winlator Bruno Dev upstream: https://github.com/brunodev85/winlator

Os arquivos em `src/` são o ponto de partida do launcher referenciado. A integração com cada runtime deve ocorrer por importação de container e seleção de binário local autorizado.

## Fork exemplar: Winlator Ludashi

O Winlator Ludashi é usado como fork exemplar no catálogo do launcher. A detecção aceita o pacote `com.winlator.ludashi`, além de nomes visíveis e pastas que contenham `winlator` ou `ludashi`. A implementação Android de referência está em `android/UniversalLauncherActivity.java`.

O repositório não redistribui o APK do Ludashi, jogos, executáveis Windows ou runtimes de terceiros. O launcher apenas detecta instalações autorizadas e mantém cada fork isolado. O índice Android aceita até 512 instalações/variantes identificadas e contabiliza diretórios únicos.

O workflow `.github/workflows/daily-upstream-status.yml` atualiza diariamente os metadados públicos dos upstreams e grava o resultado em `docs/UPSTREAM_STATUS.json`; cada alteração também aciona a publicação do site no GitHub Pages.
O limite lógico de armazenamento é **8 TB decimais** e os perfis heurísticos incluem até **24 GB de RAM**. Esses valores não alteram o tamanho do APK nem criam RAM ou armazenamento físico.
