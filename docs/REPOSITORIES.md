# Organização dos quatro projetos

1. `WinJalLauncher`: interface e importador separado.
2. `WinJalBionic_Cmod`: runtime fork público do Ludashi/Bionic.
3. `WinJalLator`: fork público do runtime base quando disponível.
4. `WinJalCmod`: fork público dedicado ao Cmod quando disponível.

Não juntar os APKs diretamente. Cada runtime mantém seu próprio package ID, assinatura, assets e bibliotecas nativas.
