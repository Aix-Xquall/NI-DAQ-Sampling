#include <iostream>
#include <vector>
#include <stdexcept>
#include <string>

#include <NIDAQmx.h>

// 簡單的錯誤檢查 macro，讓程式碼乾淨一點
#define DAQmxCheck(funcCall)                                      \
    do {                                                          \
        int32 _err = (funcCall);                                  \
        if (DAQmxFailed(_err)) {                                  \
            char _errBuff[2048] = {'\0'};                         \
            DAQmxGetExtendedErrorInfo(_errBuff, sizeof(_errBuff));\
            std::cerr << "[NI-DAQmx Error] " << _errBuff          \
                      << " (code " << _err << ")\n";              \
            throw std::runtime_error("NI-DAQmx call failed");     \
        }                                                         \
    } while (0)

// 之後可以把這個拆成 class DAQTask，不過現在先專注讓環境跑起來
int main()
{
    // ==== 1. 基本設定 ====
    // TODO: 這個 physical channel 字串要根據你實際的 cDAQ9189 與 NI-9232 名稱修改
    // 例如： "cDAQ9189-1D1234Mod1/ai0:2" 表示 Mod1 上的 ai0~ai2 三個通道
    const char* physicalChannels = "cDAQ1Mod1/ai1";

    const float64 sampleRateHz   = 1000.0;   // 1 kHz
    const int32   samplesPerRead = 100;      // 每次 Read 100 筆
    const float64 timeoutSec     = 10.0;     // 讀取 Timeout

    TaskHandle taskHandle = nullptr;

    try {
        // ==== 2. 建立 Task ====
        DAQmxCheck(DAQmxCreateTask("ai_task", &taskHandle));

        // ==== 3. 建立 AI Voltage Channel ====
        // 對 NI-9232 先當成電壓輸入，之後再細調 IEPE 等設定
        DAQmxCheck(DAQmxCreateAIVoltageChan(
            taskHandle,
            physicalChannels,     // 實體通道字串
            "",                   // 自訂通道名稱（空字串 = 使用預設）
            DAQmx_Val_Cfg_Default,
            -10.0,                // 最小量測範圍
            10.0,                 // 最大量測範圍
            DAQmx_Val_Volts,
            nullptr               // 自訂校正，不需要就 nullptr
        ));

        // ==== 4. 取樣時脈設定：內部時脈 + 1kHz + 連續取樣 ====
        DAQmxCheck(DAQmxCfgSampClkTiming(
            taskHandle,
            "",                    // 使用內部時脈
            sampleRateHz,
            DAQmx_Val_Rising,      // 上升緣觸發
            DAQmx_Val_ContSamps,   // 連續取樣
            samplesPerRead         // Buffer 大小暫設為每次讀的大小
        ));

        // ==== 5. Start Task ====
        DAQmxCheck(DAQmxStartTask(taskHandle));

        std::vector<float64> buffer(samplesPerRead);

        std::cout << "Start continuous acquisition on ["
                  << physicalChannels << "] at "
                  << sampleRateHz << " Hz...\n"
                  << "Press Ctrl+C to stop.\n";

        // ==== 6. 連續讀取迴圈 ====
        while (true) {
            int32 samplesRead = 0;

            DAQmxCheck(DAQmxReadAnalogF64(
                taskHandle,
                samplesPerRead,             // 要讀幾筆
                timeoutSec,                 // Timeout
                DAQmx_Val_GroupByScanNumber,
                buffer.data(),              // 存資料的 buffer
                static_cast<uInt32>(buffer.size()),
                &samplesRead,
                nullptr                     // 目前不需要每 channel 的有效筆數
            ));

            if (samplesRead > 0) {
                // 先簡單印第一筆，確認有數據在跳動
                std::cout << "Read " << samplesRead
                          << " samples. First sample = "
                          << buffer[0] << " V\n";
            }
        }
    }
    catch (const std::exception& ex) {
        std::cerr << "[Exception] " << ex.what() << '\n';
    }

    // ==== 7. 清理資源 ====
    if (taskHandle) {
        DAQmxStopTask(taskHandle);
        DAQmxClearTask(taskHandle);
    }

    return 0;
}
