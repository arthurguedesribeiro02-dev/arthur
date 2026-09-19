# Winlator Ludashi como fork exemplar

O launcher usa o **Winlator Ludashi** como exemplo de fork de emulador de PC Android. Ele aparece como uma variante independente; o launcher não mistura bibliotecas, containers, drivers ou runtimes entre forks.

A detecção ocorre por três sinais: nome do pacote Android, nome visível do aplicativo e diretórios externos que contenham `winlator`, `ludashi`, `runtime`, `container`, `wine` ou `box64`. A versão exibida é lida do próprio Android por meio de `PackageManager`.

O identificador conhecido incluído no exemplo é `com.winlator.ludashi`. Como forks podem mudar o `applicationId`, a detecção também aceita nomes visíveis e identificadores que contenham `winlator` ou `ludashi`.

O projeto não distribui o APK do Ludashi, arquivos de jogo, executáveis Windows ou runtimes de terceiros. O usuário deve instalar somente builds que tenha autorização para utilizar. O launcher apenas localiza o fork instalado e abre o pacote correspondente.

## Atualização semanal

O workflow `weekly-upstream-status.yml` verifica semanalmente os metadados públicos dos repositórios upstream documentados. Ele atualiza `docs/UPSTREAM_STATUS.json` com o último commit observado, sem baixar ou redistribuir APKs.

## Repositório upstream de referência

- Winlator Ludashi: <https://github.com/StevenMXZ/Winlator-Ludashi>
