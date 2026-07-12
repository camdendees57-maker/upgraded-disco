#include <jni.h>
#include <android/log.h>
#include <string>
#include <fstream>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>
#include <EGL/egl.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_android.h"
#include "imgui/imgui_impl_opengl3.h"

#define LOG_TAG "SickAssMenu"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

bool showMenu = true;
std::string nakamaToken = "";
std::string nakamaRefreshToken = "";

// ==================== FIND token.json ====================
bool findTokenFile(const std::string& path, std::string& outPath) {
    DIR* dir = opendir(path.c_str());
    if (!dir) return false;

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;
        if (name == "." || name == "..") continue;

        std::string fullPath = path + "/" + name;
        struct stat st;
        if (stat(fullPath.c_str(), &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                if (findTokenFile(fullPath, outPath)) {
                    closedir(dir);
                    return true;
                }
            } else if (name == "token.json") {
                outPath = fullPath;
                closedir(dir);
                return true;
            }
        }
    }
    closedir(dir);
    return false;
}

void loadTokenFile() {
    std::string tokenPath;
    if (!findTokenFile("/data/data", tokenPath) &&
        !findTokenFile("/sdcard", tokenPath) &&
        !findTokenFile("/storage/emulated/0", tokenPath)) {
        LOGI("token.json not found");
        return;
    }

    LOGI("Found token.json at: %s", tokenPath.c_str());

    std::ifstream file(tokenPath);
    if (!file.is_open()) return;

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    // Parse token
    size_t pos = content.find("\"token\":");
    if (pos != std::string::npos) {
        size_t start = content.find("\"", pos + 8) + 1;
        size_t end = content.find("\"", start);
        nakamaToken = content.substr(start, end - start);
    }

    // Parse refresh_token
    pos = content.find("\"refresh_token\":");
    if (pos != std::string::npos) {
        size_t start = content.find("\"", pos + 16) + 1;
        size_t end = content.find("\"", start);
        nakamaRefreshToken = content.substr(start, end - start);
    }

    if (!nakamaToken.empty()) {
        LOGI("Nakama Token Loaded");
    }
}

// ==================== ImGui MENU ====================
void RenderMenu() {
    if (!showMenu) return;

    ImGui::SetNextWindowSize(ImVec2(450, 320), ImGuiCond_FirstUseEver);
    ImGui::Begin("SickAssMenu | Animal Company");

    ImGui::Text("Token Status: %s", nakamaToken.empty() ? "Not Loaded" : "Loaded");

    if (!nakamaToken.empty()) {
        ImGui::TextWrapped("Token + Refresh Token ready for Nakama.");
    }

    ImGui::Separator();

    static bool infAmmo = false;
    static bool noRecoil = false;

    ImGui::Checkbox("Infinite Ammo", &infAmmo);
    ImGui::Checkbox("No Recoil", &noRecoil);

    ImGui::Separator();
    if (ImGui::Button("Hide Menu")) showMenu = false;

    ImGui::End();
}

// ==================== EGL HOOK ====================
static EGLBoolean (*orig_eglSwapBuffers)(EGLDisplay, EGLSurface) = nullptr;

EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplAndroid_NewFrame();
    ImGui::NewFrame();

    RenderMenu();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    return orig_eglSwapBuffers(dpy, surface);
}

// ==================== JNI ON LOAD ====================
extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    LOGI("=== SickAssMenu Native Library Loaded ===");
    loadTokenFile();

    void* egl = dlopen("libEGL.so", RTLD_NOW);
    if (egl) {
        orig_eglSwapBuffers = (EGLBoolean (*)(EGLDisplay, EGLSurface)) dlsym(egl, "eglSwapBuffers");
    }

    return JNI_VERSION_1_6;
}