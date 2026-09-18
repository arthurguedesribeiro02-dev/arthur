# Idioma do GTA V

O launcher agora oferece três fontes de idioma: **idioma do aparelho**, **país associado ao endereço IP** e **seleção manual**. O padrão é o idioma do aparelho. A consulta ao IP só acontece quando o usuário escolhe essa opção e usa `https://ipapi.co/json/`; se falhar, o launcher usa o locale local como fallback.

A preferência é salva localmente em `localStorage` com as chaves `gtav.language` e `gtav.languageSource`. O launcher não envia a preferência para um servidor próprio.

Importante: esta alteração escolhe o idioma que o launcher deve preparar para o próximo início. Ela não garante, sozinha, que o motor do GTA V alterará textos ou áudio. Para aplicar de verdade, a camada nativa/port precisa ler o código escolhido e gravar o formato de configuração que essa build do jogo suporta. Não foi possível confirmar esse formato no APK sem executar o jogo com os dados `GTAV` autorizados.
