package com.gtavsource.android;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.ActivityInfo;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.database.Cursor;
import android.graphics.Color;
import android.net.Uri;
import android.os.Bundle;
import android.os.Build;
import android.provider.DocumentsContract;
import android.provider.Settings;
import android.view.Gravity;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.Spinner;
import android.widget.TextView;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;
import java.util.Set;

public final class UniversalLauncherActivity extends Activity {
    private static final String PREFS = "universal_launcher";
    private static final String LANG = "language";
    private static final String TREE_URI = "additional_files_uri";
    // Limite lógico do índice: 8 TB decimais, sem limitar o tamanho do APK.
    private static final long MAX_STORAGE_BYTES = 8_000_000_000_000L;
    private static final int MAX_FORKS = 512;
    private static final int REQUEST_TREE = 4101;
    private static final String[] REQUIRED_DIRS = {"runtimes", "dxvk", "containers", "wine", "box64", "compression", "compression/7zip", "forks", "forks/winlator-ludashi", "forks/winlator-cmod", "forks/winlator-mali", "forks/winlator-frost", "forks/mobox", "forks/gamenative", "forks/gamehub-xiaoji", "forks/horizon", "forks/exagear", "forks/qemu", "forks/limbo", "forks/bochs", "forks/dosbox", "emuladores-pc", "emuladores-pc/winlator", "emuladores-pc/ludashi", "emuladores-pc/gamenative", "emuladores-pc/gamehub", "emuladores-pc/mobox", "emuladores-pc/exagear", "emuladores-pc/qemu", "emuladores-pc/limbo", "emuladores-pc/bochs", "emuladores-pc/dosbox"};
    private static final String[] CODES = {"pt-BR", "en", "es", "fr", "de", "it", "ru", "ar", "hi", "ja", "ko", "zh-CN"};
    private static final String[] NAMES = {"Português (Brasil)", "English", "Español", "Français", "Deutsch", "Italiano", "Русский", "العربية", "हिन्दी", "日本語", "한국어", "简体中文"};
    private static final Set<String> KNOWN = new HashSet<>(Arrays.asList(
            "com.winlator.vanilla", "com.xhynph.v.rebase", "com.winlator.cmod",
            "com.winlator", "com.winlator.frost", "com.winlator.mali", "com.winlator.ludashi",
            "com.termux", "com.termux.x11", "com.termux.api"
    ));
    private static final String[] EMULATOR_TERMS = {
            "winlator", "ludashi", "cmod", "mali", "frost", "glibc", "proton",
            "mobox", "gamehub", "xiaoji", "gamenative", "horizon", "termux", "exagear", "wine", "box64",
            "box86", "qemu", "limbo", "bochs", "dosbox", "7zip", "onexplayer", "emulator",
            "pc emulator", "pcem", "virtuabox", "virtualbox", "andronix"
    };
    private static final String[] REPOSITORY_NAMES = {"ArthurJAL / Launcher", "WinJalBionic Cmod", "Winlator Ludashi", "Winlator Cmod", "Winlator Vanilla", "GameNative", "GameHub / Xiaoji", "CPUID oficial", "CPU-Z oficial", "CPU-Z Android / Play Store", "7-Zip oficial"};
    private static final String[] REPOSITORY_URLS = {
            "https://github.com/arthurguedesribeiro02-dev/arthur",
            "https://github.com/arthurguedesribeiro02-dev/WinJalBionic_Cmod",
            "https://github.com/StevenMXZ/Winlator-Ludashi",
            "https://github.com/coffincolors/winlator",
            "https://github.com/brunodev85/winlator",
            "https://gamenative.app/",
            "https://gamehub.xiaoji.com/en/",
            "https://www.cpuid.com/",
            "https://www.cpuid.com/softwares/cpu-z.html",
            "https://play.google.com/store/apps/details?id=com.cpuid.cpu_z&hl=pt_BR",
            "https://www.7-zip.org/"
    };

    private SharedPreferences prefs;
    private LinearLayout list;
    private TextView storageStatus;
    private Spinner language;

    @Override public void onCreate(Bundle state) {
        super.onCreate(state);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        prefs = getSharedPreferences(PREFS, MODE_PRIVATE);
        buildUi();
        String savedTree = prefs.getString(TREE_URI, null);
        if (savedTree != null) prepareRequiredFolders(Uri.parse(savedTree));
        if (prefs.getString(TREE_URI, null) == null) {
            new android.os.Handler().postDelayed(this::chooseDataFolder, 450);
        }
    }

    private String lang() {
        String saved = prefs.getString(LANG, null);
        if (saved != null) return saved;
        String detected = detectLanguage();
        prefs.edit().putString(LANG, detected).apply();
        return detected;
    }

    private String detectLanguage() {
        String code = Locale.getDefault().getLanguage().toLowerCase(Locale.ROOT);
        if ("pt".equals(code)) return "pt-BR";
        if ("en".equals(code)) return "en";
        if ("es".equals(code)) return "es";
        if ("fr".equals(code)) return "fr";
        if ("de".equals(code)) return "de";
        if ("it".equals(code)) return "it";
        if ("ru".equals(code)) return "ru";
        if ("ar".equals(code)) return "ar";
        if ("hi".equals(code)) return "hi";
        if ("ja".equals(code)) return "ja";
        if ("ko".equals(code)) return "ko";
        if ("zh".equals(code)) return "zh-CN";
        return "pt-BR";
    }

    private String tr(String pt, String en) {
        return "pt-BR".equals(lang()) ? pt : en;
    }

    private TextView text(String value, int size) {
        TextView v = new TextView(this);
        v.setText(value); v.setTextSize(size); v.setTextColor(Color.WHITE); v.setPadding(20, 12, 20, 12);
        return v;
    }

    private void buildUi() {
        LinearLayout root = new LinearLayout(this); root.setOrientation(LinearLayout.VERTICAL); root.setPadding(24, 18, 24, 18); root.setBackgroundColor(Color.rgb(18, 20, 24));
        TextView title = text(tr("GTA V — Pro Launcher", "GTA V — Pro Launcher"), 24); title.setTextColor(Color.rgb(120, 190, 255)); root.addView(title, new LinearLayout.LayoutParams(-1, -2));
        TextView intro = text(tr("Launcher universal para Winlator, Winlator Ludashi, Mobox e outros forks. Na primeira abertura, selecione a pasta do sistema/dados para localizar runtimes, containers e DXVK. No Android recente, Android/data e Android/obb são protegidos pelo sistema e podem não aparecer no seletor.", "Universal launcher for Winlator, Winlator Ludashi, Mobox and other forks. On first launch, select the system/data folder to find runtimes, containers and DXVK. On recent Android versions, Android/data and Android/obb are protected by the system and may not appear in the picker."), 14); root.addView(intro, new LinearLayout.LayoutParams(-1, -2));
        TextView device = text(deviceSummary(), 14); device.setTextColor(Color.rgb(180, 210, 235)); root.addView(device, new LinearLayout.LayoutParams(-1, -2));
        LinearLayout langRow = new LinearLayout(this); langRow.setGravity(Gravity.CENTER_VERTICAL); TextView langLabel = text(tr("Idioma", "Language"), 16); langRow.addView(langLabel, new LinearLayout.LayoutParams(0, -2, 1));
        language = new Spinner(this); ArrayAdapter<String> adapter = new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, NAMES); language.setAdapter(adapter); int selected = 0; for (int i=0;i<CODES.length;i++) if (CODES[i].equals(lang())) selected=i; language.setSelection(selected);
        language.setOnItemSelectedListener(new android.widget.AdapterView.OnItemSelectedListener() { public void onNothingSelected(android.widget.AdapterView<?> p) {} public void onItemSelected(android.widget.AdapterView<?> p, View v, int pos, long id) { if (!CODES[pos].equals(lang())) { prefs.edit().putString(LANG, CODES[pos]).apply(); buildUi(); } } });
        langRow.addView(language, new LinearLayout.LayoutParams(260, -2)); root.addView(langRow);
        Button folder = new Button(this); folder.setText(tr("Selecionar pasta do sistema/dados", "Select system/data folder")); folder.setOnClickListener(v -> chooseDataFolder()); root.addView(folder, new LinearLayout.LayoutParams(-1, -2));
        storageStatus = text(tr("Pasta de dados: não selecionada\nLimite do launcher: 8 TB\nLimite de forks: 512", "Data folder: not selected\nLauncher limit: 8 TB\nFork limit: 512"), 15); root.addView(storageStatus, new LinearLayout.LayoutParams(-1, -2));
        Button refresh = new Button(this); refresh.setText(tr("Atualizar espaço e forks", "Refresh storage and forks")); refresh.setOnClickListener(v -> refreshExternalData()); root.addView(refresh, new LinearLayout.LayoutParams(-1, -2));
        Button repos = new Button(this); repos.setText(tr("Abrir repositórios oficiais", "Open official repositories")); repos.setOnClickListener(v -> showRepositories()); root.addView(repos, new LinearLayout.LayoutParams(-1, -2));
        Button gta = new Button(this); gta.setText(tr("Abrir GTA V", "Open GTA V")); gta.setOnClickListener(v -> { try { Intent i = new Intent(); i.setClassName(getPackageName(), "com.gtavsource.android.StoragePermissionActivity"); startActivity(i); } catch (Exception ignored) {} }); root.addView(gta, new LinearLayout.LayoutParams(-1, -2));
        TextView hint = text(tr("Emuladores e forks instalados", "Installed emulators and forks"), 16); root.addView(hint);
        ScrollView scroll = new ScrollView(this); list = new LinearLayout(this); list.setOrientation(LinearLayout.VERTICAL); scroll.addView(list); root.addView(scroll, new LinearLayout.LayoutParams(-1, 0, 1));
        setContentView(root); scanApps(); refreshExternalData();
    }

    private void showRepositories() {
        new AlertDialog.Builder(this).setTitle(tr("Repositórios e projetos", "Repositories and projects"))
                .setItems(REPOSITORY_NAMES, (dialog, which) -> { try { startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse(REPOSITORY_URLS[which]))); } catch (Exception ignored) {} }).show();
    }

    private String deviceSummary() {
        String model = Build.MANUFACTURER + " " + Build.MODEL;
        String abi = Build.SUPPORTED_ABIS != null && Build.SUPPORTED_ABIS.length > 0 ? Build.SUPPORTED_ABIS[0] : "unknown";
        boolean pocoF5 = model.toLowerCase(Locale.ROOT).contains("poco f5") || model.toLowerCase(Locale.ROOT).contains("23049pcd8g");
        String profile = pocoF5 ? tr("Perfil: POCO F5 / Snapdragon 7+ Gen 2 / Adreno / ARM64", "Profile: POCO F5 / Snapdragon 7+ Gen 2 / Adreno / ARM64") : tr("Perfil automático do aparelho", "Automatic device profile");
        return tr("Aparelho: ", "Device: ") + model + "\n" + tr("Arquitetura: ", "Architecture: ") + abi + " · Android " + Build.VERSION.SDK_INT + "\n" + profile;
    }

    private void chooseDataFolder() {
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE); intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION | Intent.FLAG_GRANT_PERSISTABLE_URI_PERMISSION | Intent.FLAG_GRANT_PREFIX_URI_PERMISSION); startActivityForResult(intent, REQUEST_TREE);
    }

    @Override protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == REQUEST_TREE && resultCode == RESULT_OK && data != null && data.getData() != null) {
            Uri uri = data.getData(); try { getContentResolver().takePersistableUriPermission(uri, Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION); } catch (Exception ignored) {}
            prefs.edit().putString(TREE_URI, uri.toString()).apply(); prepareRequiredFolders(uri); refreshExternalData();
        }
    }

    private void prepareRequiredFolders(Uri tree) {
        new Thread(() -> {
            int created = 0;
            for (String name : REQUIRED_DIRS) {
                try {
                    String parentId = DocumentsContract.getTreeDocumentId(tree);
                    String parentPath = name.contains("/") ? name.substring(0, name.lastIndexOf('/')) : null;
                    if (parentPath != null) {
                        for (String part : parentPath.split("/")) {
                            Uri parentChildren = DocumentsContract.buildChildDocumentsUriUsingTree(tree, parentId);
                            Cursor pc = getContentResolver().query(parentChildren, new String[]{DocumentsContract.Document.COLUMN_DOCUMENT_ID, DocumentsContract.Document.COLUMN_DISPLAY_NAME}, "display_name=?", new String[]{part}, null);
                            if (pc == null || !pc.moveToFirst()) { if (pc != null) pc.close(); parentId = null; break; }
                            parentId = pc.getString(0); pc.close();
                        }
                    }
                    if (parentId == null) continue;
                    Uri child = DocumentsContract.buildChildDocumentsUriUsingTree(tree, parentId);
                    Cursor c = getContentResolver().query(child, new String[]{DocumentsContract.Document.COLUMN_DISPLAY_NAME}, "display_name=?", new String[]{name}, null);
                    boolean exists = c != null && c.moveToFirst(); if (c != null) c.close();
                    if (!exists && DocumentsContract.createDocument(getContentResolver(), DocumentsContract.buildDocumentUriUsingTree(tree, parentId), "vnd.android.document/directory", name.substring(name.lastIndexOf('/') + 1)) != null) created++;
                } catch (Exception ignored) { }
            }
            final int made = created;
            runOnUiThread(() -> storageStatus.setText(tr("Pasta preparada. Diretórios criados: ", "Folder prepared. Directories created: ") + made + "/" + REQUIRED_DIRS.length + tr("\nOs runtimes devem ser fornecidos separadamente.", "\nRuntimes must be provided separately.")));
        }).start();
    }

    private void refreshExternalData() {
        if (storageStatus == null) return;
        final String value = prefs.getString(TREE_URI, null);
        if (value == null) { storageStatus.setText(tr("Pasta de dados: não selecionada\nLimite do launcher: 8 TB\nLimite de forks: 512", "Data folder: not selected\nLauncher limit: 8 TB\nFork limit: 512")); scanApps(); return; }
        storageStatus.setText(tr("Lendo pasta selecionada...\nLimite do launcher: 8 TB\nLimite de forks: 512", "Reading selected folder...\nLauncher limit: 8 TB\nFork limit: 512"));
        new Thread(() -> { ScanResult result = scanTree(Uri.parse(value)); runOnUiThread(() -> { String state = result.bytes > MAX_STORAGE_BYTES ? tr("LIMITE EXCEDIDO", "LIMIT EXCEEDED") : tr("dentro do limite", "within limit"); String note = result.truncated ? tr("\nVarredura interrompida no limite de segurança; atualize por subpastas.", "\nScan stopped at the safety limit; refresh by subfolder.") : ""; storageStatus.setText(tr("Pasta de dados: selecionada\nTamanho indexado: ", "Data folder: selected\nIndexed size: ") + formatBytes(result.bytes) + tr("\nForks encontrados: ", "\nForks found: ") + result.forkIds.size() + "/" + MAX_FORKS + tr("\nEstado: ", "\nStatus: ") + state + note); scanApps(); }); }).start();
    }

    private static final class ScanResult { long bytes; long nodes; boolean truncated; final Set<String> forkIds = new HashSet<>(); }
    private ScanResult scanTree(Uri tree) {
        ScanResult out = new ScanResult();
        String root = DocumentsContract.getTreeDocumentId(tree);
        if (root == null) return out;
        ArrayList<String> stack = new ArrayList<>(); stack.add(root);
        final int[] projection = {0};
        while (!stack.isEmpty() && !out.truncated) {
            String documentId = stack.remove(stack.size() - 1);
            Uri children = DocumentsContract.buildChildDocumentsUriUsingTree(tree, documentId);
            Cursor c = null;
            try {
                c = getContentResolver().query(children, new String[]{
                        DocumentsContract.Document.COLUMN_DOCUMENT_ID,
                        DocumentsContract.Document.COLUMN_DISPLAY_NAME,
                        DocumentsContract.Document.COLUMN_MIME_TYPE,
                        DocumentsContract.Document.COLUMN_SIZE
                }, null, null, null);
                if (c == null) continue;
                int idCol = c.getColumnIndex(DocumentsContract.Document.COLUMN_DOCUMENT_ID);
                int nameCol = c.getColumnIndex(DocumentsContract.Document.COLUMN_DISPLAY_NAME);
                int mimeCol = c.getColumnIndex(DocumentsContract.Document.COLUMN_MIME_TYPE);
                int sizeCol = c.getColumnIndex(DocumentsContract.Document.COLUMN_SIZE);
                while (c.moveToNext()) {
                    if (++out.nodes > 1000000L) { out.truncated = true; break; }
                    String id = c.getString(idCol); String name = c.getString(nameCol); String mime = c.getString(mimeCol);
                    if (DocumentsContract.Document.MIME_TYPE_DIR.equals(mime)) {
                        if (isForkName(name) && id != null && out.forkIds.size() < MAX_FORKS) out.forkIds.add(id);
                        if (id != null) stack.add(id);
                    } else if (sizeCol >= 0 && !c.isNull(sizeCol)) {
                        long size = Math.max(0L, c.getLong(sizeCol));
                        if (out.bytes > Long.MAX_VALUE - size) { out.truncated = true; break; }
                        out.bytes += size;
                    }
                }
            } catch (Exception ignored) { }
            finally { if (c != null) c.close(); }
        }
        return out;
    }
    private boolean isForkName(String name) { return matchesEmulatorTerm(name); }
    private boolean matchesEmulatorTerm(String value) {
        if (value == null) return false;
        String n = value.toLowerCase(Locale.ROOT);
        for (String term : EMULATOR_TERMS) if (n.contains(term)) return true;
        return n.contains("runtime") || n.contains("container") || n.contains("wineprefix");
    }
    private String formatBytes(long bytes) { double v=bytes; String[] units={"B","KB","MB","GB","TB"}; int i=0; while(v>=1024 && i<units.length-1){v/=1024;i++;} return String.format(Locale.US,"%.2f %s",v,units[i]); }

    private void scanApps() {
        if (list == null) return;
        list.removeAllViews();
        PackageManager pm = getPackageManager(); Intent query = new Intent(Intent.ACTION_MAIN); query.addCategory(Intent.CATEGORY_LAUNCHER); List<ResolveInfo> infos = pm.queryIntentActivities(query, PackageManager.MATCH_ALL); List<ApplicationInfo> apps = new ArrayList<>();
        for (ResolveInfo ri : infos) { ApplicationInfo ai=ri.activityInfo.applicationInfo; String pkg=ai.packageName; String label=String.valueOf(pm.getApplicationLabel(ai)); String s=(pkg+" "+label).toLowerCase(Locale.ROOT); boolean system=(ai.flags & ApplicationInfo.FLAG_SYSTEM) != 0; if (!getPackageName().equals(pkg) && !system && (KNOWN.contains(pkg) || matchesEmulatorTerm(s))) apps.add(ai); }
        Collections.sort(apps, new Comparator<ApplicationInfo>() { public int compare(ApplicationInfo a, ApplicationInfo b) { return String.valueOf(pm.getApplicationLabel(a)).compareToIgnoreCase(String.valueOf(pm.getApplicationLabel(b))); } });
        if (apps.isEmpty()) { list.addView(text(tr("Nenhum fork ou emulador de PC instalado foi encontrado.", "No installed PC fork or emulator was found."), 16)); return; }
        int count=0; for (ApplicationInfo ai : apps) { if (count++ >= MAX_FORKS) break; addCard(pm, ai); }
    }

    private void addCard(PackageManager pm, ApplicationInfo ai) {
        LinearLayout card=new LinearLayout(this); card.setOrientation(LinearLayout.VERTICAL); card.setPadding(18,12,18,12); card.setBackgroundColor(Color.rgb(35,39,46)); String name=String.valueOf(pm.getApplicationLabel(ai)); String version=""; try { version=pm.getPackageInfo(ai.packageName,0).versionName; } catch(Exception ignored) {}
        TextView info=text(name+"\n"+ai.packageName+(version==null?"":"  v"+version),17); card.addView(info,new LinearLayout.LayoutParams(-1,-2)); Button open=new Button(this); open.setText("Abrir"); open.setOnClickListener(v->{Intent i=pm.getLaunchIntentForPackage(ai.packageName);if(i!=null)startActivity(i);}); card.addView(open,new LinearLayout.LayoutParams(-1,-2)); LinearLayout.LayoutParams cp=new LinearLayout.LayoutParams(-1,-2); cp.setMargins(0,0,0,12); list.addView(card,cp);
    }
}
