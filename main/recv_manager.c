#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "nvs_flash.h"
#include "esp_mac.h"
#include "rom/ets_sys.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_now.h"

static const char *csi_recv_tag = "recv_manager";

#define CONFIG_LESS_INTERFERENCE_CHANNEL    11
#define CONFIG_SEND_FREQUENCY               100

#define MAC_STR_TO_HEX_ARRAY(mac_str, mac_hex) \
sscanf((mac_str), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &(mac_hex)[0], &(mac_hex)[1], &(mac_hex)[2], &(mac_hex)[3], &(mac_hex)[4], &(mac_hex)[5])

static uint8_t CSI_SEND_MAC[6];

void csi_recv_cb(void *ctx, wifi_csi_info_t *info)
{
    MAC_STR_TO_HEX_ARRAY(CONFIG_CSI_SEND_MAC, CSI_SEND_MAC);

    if (!info || !info->buf)
    {
        ESP_LOGW(csi_recv_tag, "<%s> wifi_csi_cb", esp_err_to_name(ESP_ERR_INVALID_ARG));
        return;
    }

    if (memcmp(info->mac, CSI_SEND_MAC, 6))
    {
        return;
    }

    static int s_count = 0;
    const wifi_pkt_rx_ctrl_t *rx_ctrl = &info->rx_ctrl;

    if (!s_count)
    {
        ESP_LOGI(csi_recv_tag, "================ CSI RECV ================");
        ets_printf("type,id,mac,rssi,rate,sig_mode,mcs,bandwidth,smoothing,not_sounding,aggregation,stbc,fec_coding,sgi,noise_floor,ampdu_cnt,channel,secondary_channel,local_timestamp,ant,sig_len,rx_state,len,first_word,data\n");
    }

    ets_printf("CSI_DATA,%d," MACSTR ",%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
            s_count++, MAC2STR(info->mac), rx_ctrl->rssi, rx_ctrl->rate, rx_ctrl->sig_mode,
            rx_ctrl->mcs, rx_ctrl->cwb, rx_ctrl->smoothing, rx_ctrl->not_sounding,
            rx_ctrl->aggregation, rx_ctrl->stbc, rx_ctrl->fec_coding, rx_ctrl->sgi,
            rx_ctrl->noise_floor, rx_ctrl->ampdu_cnt, rx_ctrl->channel, rx_ctrl->secondary_channel,
            rx_ctrl->timestamp, rx_ctrl->ant, rx_ctrl->sig_len, rx_ctrl->rx_state);

    ets_printf(",%d,%d,\"[%d", info->len, info->first_word_invalid, info->buf[0]);

    for (int i = 1; i < info->len; i++)
    {
        ets_printf(",%d", info->buf[i]);
    }

    ets_printf("]\"\n");
}