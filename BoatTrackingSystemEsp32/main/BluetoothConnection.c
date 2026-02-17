
#include "BluetoothConnection.h"
#include "BluetoothUUIDInfo.h" //Be sure to create a new BluetoothUUIDInfo.h and then add each uuid to the header

extern const ble_uuid128_t gatt_svr_svc_uuid;

static uint8_t gatt_svr_chr_val;
static uint16_t gatt_svr_chr_val_handle;
      


/* A characteristic that can be subscribed to */
static uint8_t gatt_svr_chr_val;
static uint16_t gatt_svr_chr_val_handle;

extern const ble_uuid128_t gatt_svr_chr_uuid;

/* A custom descriptor */
static uint8_t gatt_svr_dsc_val;
static const ble_uuid128_t gatt_svr_dsc_uuid ;

static int
gatt_svc_access(uint16_t conn_handle, uint16_t attr_handle,
                struct ble_gatt_access_ctxt *ctxt,
                void *arg);

static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    {
        /*** Service ***/
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &gatt_svr_svc_uuid.u,
        
        .characteristics = (struct ble_gatt_chr_def[])
        { {
                /*** This characteristic can be subscribed to by writing 0x00 and 0x01 to the CCCD ***/
                .uuid = &gatt_svr_chr_uuid.u,
                .access_cb = gatt_svc_access,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
                .val_handle = &gatt_svr_chr_val_handle,
                
                // .descriptors = (struct ble_gatt_dsc_def[])
                // { {
                //       .uuid = &gatt_svr_dsc_uuid.u,

                //       .att_flags = BLE_ATT_F_READ,
                //       .access_cb = gatt_svc_access,
                //     }, {
                //       0, /* No more descriptors in this characteristic */
                //     }
                // },
            }, {
                0, /* No more characteristics in this service. */
            }
        },
    },

    {
        0, /* No more services. */
    },
};

static int
gatt_svr_write(struct os_mbuf *om, uint16_t min_len, uint16_t max_len,
               void *dst, uint16_t *len)
{
    uint16_t om_len;
    int rc;

    om_len = OS_MBUF_PKTLEN(om);
    if (om_len < min_len || om_len > max_len) {
        return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
    }

    rc = ble_hs_mbuf_to_flat(om, dst, max_len, len);
    if (rc != 0) {
        return BLE_ATT_ERR_UNLIKELY;
    }

    return 0;
}

/**
 * Access callback whenever a characteristic/descriptor is read or written to.
 * Here reads and writes need to be handled.
 * ctxt->op tells weather the operation is read or write and
 * weather it is on a characteristic or descriptor,
 * ctxt->dsc->uuid tells which characteristic/descriptor is accessed.
 * attr_handle give the value handle of the attribute being accessed.
 * Accordingly do:
 *     Append the value to ctxt->om if the operation is READ
 *     Write ctxt->om to the value if the operation is WRITE
 **/
static int
gatt_svc_access(uint16_t conn_handle, uint16_t attr_handle,
                struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    const ble_uuid_t *uuid;
    int rc;

    switch (ctxt->op) {
    case BLE_GATT_ACCESS_OP_READ_CHR:
        if (conn_handle != BLE_HS_CONN_HANDLE_NONE) {
            MODLOG_DFLT(INFO, "Characteristic read; conn_handle=%d attr_handle=%d\n",
                        conn_handle, attr_handle);
        } else {
            MODLOG_DFLT(INFO, "Characteristic read by NimBLE stack; attr_handle=%d\n",
                        attr_handle);
        }
        uuid = ctxt->chr->uuid;
        if (attr_handle == gatt_svr_chr_val_handle) {
            rc = os_mbuf_append(ctxt->om,
                                get_gps_data_ptr(),
                                (sizeof(GpsData)));
            return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
        }
        goto unknown;
        /*
    case BLE_GATT_ACCESS_OP_WRITE_CHR:
        if (conn_handle != BLE_HS_CONN_HANDLE_NONE) {
            MODLOG_DFLT(INFO, "Characteristic write; conn_handle=%d attr_handle=%d",
                        conn_handle, attr_handle);
        } else {
            MODLOG_DFLT(INFO, "Characteristic write by NimBLE stack; attr_handle=%d",
                        attr_handle);
        }
        uuid = ctxt->chr->uuid;
        if (attr_handle == gatt_svr_chr_val_handle) {
            rc = gatt_svr_write(ctxt->om,
                                sizeof(gatt_svr_chr_val),
                                sizeof(gatt_svr_chr_val),
                                &gatt_svr_chr_val, NULL);
            ble_gatts_chr_updated(attr_handle);
            MODLOG_DFLT(INFO, "Notification/Indication scheduled for "
                        "all subscribed peers.\n");
            return rc;
        }
        goto unknown;
        */

    case BLE_GATT_ACCESS_OP_READ_DSC:
        if (conn_handle != BLE_HS_CONN_HANDLE_NONE) {
            MODLOG_DFLT(INFO, "Descriptor read; conn_handle=%d attr_handle=%d\n",
                        conn_handle, attr_handle);
        } else {
            MODLOG_DFLT(INFO, "Descriptor read by NimBLE stack; attr_handle=%d\n",
                        attr_handle);
        }
        uuid = ctxt->dsc->uuid;
        if (ble_uuid_cmp(uuid, &gatt_svr_dsc_uuid.u) == 0) {
            rc = os_mbuf_append(ctxt->om,
                                &gatt_svr_dsc_val,
                                sizeof(gatt_svr_chr_val));
            return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
        }
        goto unknown;

    case BLE_GATT_ACCESS_OP_WRITE_DSC:
        goto unknown;

    default:
        goto unknown;
    }

unknown:
    /* Unknown characteristic/descriptor;
     * The NimBLE host should not have called this function;
     */
    assert(0);
    return BLE_ATT_ERR_UNLIKELY;
}

void
gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg)
{
    char buf[BLE_UUID_STR_LEN];

    switch (ctxt->op) {
    case BLE_GATT_REGISTER_OP_SVC:
        MODLOG_DFLT(DEBUG, "registered service %s with handle=%d\n",
                    ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf),
                    ctxt->svc.handle);
        break;

    case BLE_GATT_REGISTER_OP_CHR:
        MODLOG_DFLT(DEBUG, "registering characteristic %s with "
                    "def_handle=%d val_handle=%d\n",
                    ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf),
                    ctxt->chr.def_handle,
                    ctxt->chr.val_handle);
        break;

    case BLE_GATT_REGISTER_OP_DSC:
        MODLOG_DFLT(DEBUG, "registering descriptor %s with handle=%d\n",
                    ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf),
                    ctxt->dsc.handle);
        break;

    default:
        assert(0);
        break;
    }
}

int
gatt_svr_init(void)
{
    int rc;

    ble_svc_gap_init();
    ble_svc_gatt_init();
   // ble_svc_ans_init();

    rc = ble_gatts_count_cfg(gatt_svr_svcs);
    if (rc != 0) {
        return rc;
    }

    rc = ble_gatts_add_svcs(gatt_svr_svcs);
    if (rc != 0) {
        return rc;
    }

    /* Setting a value for the read-only descriptor */
    gatt_svr_dsc_val = 0x99;

    return 0;
}

///////////////////////

#if CONFIG_EXAMPLE_EXTENDED_ADV
static uint8_t ext_adv_pattern_1[] = {
    0x02, 0x01, 0x06,
    0x03, 0x03, 0xab, 0xcd,
    0x03, 0x03, 0x18, 0x11,
    0x11, 0X09, 'n', 'i', 'm', 'b', 'l', 'e', '-', 'b', 'l', 'e', 'p', 'r', 'p', 'h', '-', 'e',
};
#endif

static const char *tag = "NimBLE_BLE_PRPH";
static int bleprph_gap_event(struct ble_gap_event *event, void *arg);
#if CONFIG_EXAMPLE_RANDOM_ADDR
static uint8_t own_addr_type = BLE_OWN_ADDR_RANDOM;
#else
static uint8_t own_addr_type;
#endif

#if MYNEWT_VAL(BLE_EATT_CHAN_NUM) > 0
static uint16_t cids[MYNEWT_VAL(BLE_EATT_CHAN_NUM)];
static uint16_t bearers;
#endif

void ble_store_config_init(void);

//#if NIMBLE_BLE_CONNECT
/**
 * Logs information about a connection to the console.
 */
static void
bleprph_print_conn_desc(struct ble_gap_conn_desc *desc)
{
    MODLOG_DFLT(INFO, "handle=%d our_ota_addr_type=%d our_ota_addr=",
                desc->conn_handle, desc->our_ota_addr.type);
    print_addr(desc->our_ota_addr.val);
    MODLOG_DFLT(INFO, " our_id_addr_type=%d our_id_addr=",
                desc->our_id_addr.type);
    print_addr(desc->our_id_addr.val);
    MODLOG_DFLT(INFO, " peer_ota_addr_type=%d peer_ota_addr=",
                desc->peer_ota_addr.type);
    print_addr(desc->peer_ota_addr.val);
    MODLOG_DFLT(INFO, " peer_id_addr_type=%d peer_id_addr=",
                desc->peer_id_addr.type);
    print_addr(desc->peer_id_addr.val);
    MODLOG_DFLT(INFO, " conn_itvl=%d conn_latency=%d supervision_timeout=%d "
                "encrypted=%d authenticated=%d bonded=%d\n",
                desc->conn_itvl, desc->conn_latency,
                desc->supervision_timeout,
                desc->sec_state.encrypted,
                desc->sec_state.authenticated,
                desc->sec_state.bonded);
}

/**
 * Enables advertising with the following parameters:
 *     o General discoverable mode.
 *     o Undirected connectable mode.
 */
static void
bleprph_advertise(void)
{
    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;

    const char *name ;
    int rc;

    /**
     *  Set the advertisement data included in our advertisements:
     *     o Flags (indicates advertisement type and other general info).
     *     o Advertising tx power.
     *     o Device name.
     *     o 16-bit service UUIDs (alert notifications).
     */

    memset(&fields, 0, sizeof fields);

    /* Advertise two flags:
     *     o Discoverability in forthcoming advertisement (general)
     *     o BLE-only (BR/EDR unsupported).
     */
    fields.flags = BLE_HS_ADV_F_DISC_GEN |
                   BLE_HS_ADV_F_BREDR_UNSUP;

    /* Indicate that the TX power level field should be included; have the
     * stack fill this value automatically.  This is done by assigning the
     * special value BLE_HS_ADV_TX_PWR_LVL_AUTO.
     */
    fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;


    name = ble_svc_gap_device_name();
    fields.name = (uint8_t *)name;
    fields.name_len = strlen(name);
    fields.name_is_complete = 1;


    rc = ble_gap_adv_set_fields(&fields);

    // fields.uuids16 = (ble_uuid16_t[]) {
    //     BLE_UUID16_INIT(GATT_SVR_SVC_ALERT_UUID)
    // };
    // fields.num_uuids16 = 1;
    // fields.uuids16_is_complete = 1;

    //name of 
    fields.uuids128 = (ble_uuid128_t[]) {
        gatt_svr_svc_uuid
    };
    fields.num_uuids128 = 1;
    fields.uuids128_is_complete = 1;



struct ble_hs_adv_fields scan_rsp;
memset(&scan_rsp, 0, sizeof(scan_rsp));

scan_rsp.uuids128 = (ble_uuid128_t[]) {
    gatt_svr_svc_uuid
};
scan_rsp.num_uuids128 = 1;
    scan_rsp.uuids128_is_complete = 1;

    ble_gap_adv_rsp_set_fields(&scan_rsp);

    
    /* Begin advertising. */
    memset(&adv_params, 0, sizeof adv_params);
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER,
                           &adv_params, bleprph_gap_event, NULL);
}


#if MYNEWT_VAL(BLE_POWER_CONTROL)
static void bleprph_power_control(uint16_t conn_handle)
{
    int rc;

    rc = ble_gap_read_remote_transmit_power_level(conn_handle, 0x01 );  // Attempting on LE 1M phy
    assert (rc == 0);

    rc = ble_gap_set_transmit_power_reporting_enable(conn_handle, 0x1, 0x1);
    assert (rc == 0);
}
#endif

/**
 * The nimble host executes this callback when a GAP event occurs.  The
 * application associates a GAP event callback with each connection that forms.
 * bleprph uses the same callback for all connections.
 *
 * @param event                 The type of event being signalled.
 * @param ctxt                  Various information pertaining to the event.
 * @param arg                   Application-specified argument; unused by
 *                                  bleprph.
 *
 * @return                      0 if the application successfully handled the
 *                                  event; nonzero on failure.  The semantics
 *                                  of the return code is specific to the
 *                                  particular GAP event being signalled.
 */
static int
bleprph_gap_event(struct ble_gap_event *event, void *arg)
{
    struct ble_gap_conn_desc desc;
    int rc;

    switch (event->type) {

    case BLE_GAP_EVENT_CONNECT:
        /* A new connection was established or a connection attempt failed. */
        MODLOG_DFLT(INFO, "connection %s; status=%d ",
                    event->connect.status == 0 ? "established" : "failed",
                    event->connect.status);
        if (event->connect.status == 0) {
            rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
            assert(rc == 0);
            bleprph_print_conn_desc(&desc);
        }
        MODLOG_DFLT(INFO, "\n");

        if (event->connect.status != 0) {
            /* Connection failed; resume advertising. */
#if CONFIG_EXAMPLE_EXTENDED_ADV
            ext_bleprph_advertise();
#else
            bleprph_advertise();
#endif
        }

#if MYNEWT_VAL(BLE_POWER_CONTROL)
	bleprph_power_control(event->connect.conn_handle);
#endif
        return 0;

    case BLE_GAP_EVENT_DISCONNECT:
        MODLOG_DFLT(INFO, "disconnect; reason=%d ", event->disconnect.reason);
        bleprph_print_conn_desc(&event->disconnect.conn);
        MODLOG_DFLT(INFO, "\n");

        /* Connection terminated; resume advertising. */
#if CONFIG_EXAMPLE_EXTENDED_ADV
        ext_bleprph_advertise();
#else
        bleprph_advertise();
#endif
        return 0;

    case BLE_GAP_EVENT_CONN_UPDATE:
        /* The central has updated the connection parameters. */
        MODLOG_DFLT(INFO, "connection updated; status=%d ",
                    event->conn_update.status);
        rc = ble_gap_conn_find(event->conn_update.conn_handle, &desc);
        assert(rc == 0);
        bleprph_print_conn_desc(&desc);
        MODLOG_DFLT(INFO, "\n");
        return 0;

    case BLE_GAP_EVENT_ADV_COMPLETE:
        MODLOG_DFLT(INFO, "advertise complete; reason=%d",
                    event->adv_complete.reason);
#if CONFIG_EXAMPLE_EXTENDED_ADV
        ext_bleprph_advertise();
#else
        bleprph_advertise();
#endif
        return 0;

    case BLE_GAP_EVENT_ENC_CHANGE:
        /* Encryption has been enabled or disabled for this connection. */
        MODLOG_DFLT(INFO, "encryption change event; status=%d ",
                    event->enc_change.status);
        rc = ble_gap_conn_find(event->enc_change.conn_handle, &desc);
        assert(rc == 0);
        bleprph_print_conn_desc(&desc);
        MODLOG_DFLT(INFO, "\n");
        return 0;

    case BLE_GAP_EVENT_NOTIFY_TX:
        MODLOG_DFLT(INFO, "notify_tx event; conn_handle=%d attr_handle=%d "
                    "status=%d is_indication=%d",
                    event->notify_tx.conn_handle,
                    event->notify_tx.attr_handle,
                    event->notify_tx.status,
                    event->notify_tx.indication);
        return 0;

    case BLE_GAP_EVENT_SUBSCRIBE:
        MODLOG_DFLT(INFO, "subscribe event; conn_handle=%d attr_handle=%d "
                    "reason=%d prevn=%d curn=%d previ=%d curi=%d\n",
                    event->subscribe.conn_handle,
                    event->subscribe.attr_handle,
                    event->subscribe.reason,
                    event->subscribe.prev_notify,
                    event->subscribe.cur_notify,
                    event->subscribe.prev_indicate,
                    event->subscribe.cur_indicate);
        return 0;

    case BLE_GAP_EVENT_MTU:
        MODLOG_DFLT(INFO, "mtu update event; conn_handle=%d cid=%d mtu=%d\n",
                    event->mtu.conn_handle,
                    event->mtu.channel_id,
                    event->mtu.value);
        return 0;

    case BLE_GAP_EVENT_REPEAT_PAIRING:
        /* We already have a bond with the peer, but it is attempting to
         * establish a new secure link.  This app sacrifices security for
         * convenience: just throw away the old bond and accept the new link.
         */

        /* Delete the old bond. */
        rc = ble_gap_conn_find(event->repeat_pairing.conn_handle, &desc);
        assert(rc == 0);
        ble_store_util_delete_peer(&desc.peer_id_addr);

        /* Return BLE_GAP_REPEAT_PAIRING_RETRY to indicate that the host should
         * continue with the pairing operation.
         */
        return BLE_GAP_REPEAT_PAIRING_RETRY;

    case BLE_GAP_EVENT_PASSKEY_ACTION:
        ESP_LOGI(tag, "PASSKEY_ACTION_EVENT started");
        struct ble_sm_io pkey = {0};
        int key = 0;

        if (event->passkey.params.action == BLE_SM_IOACT_DISP) {
            pkey.action = event->passkey.params.action;
            pkey.passkey = 123456; // This is the passkey to be entered on peer
            ESP_LOGI(tag, "Enter passkey %" PRIu32 "on the peer side", pkey.passkey);
            rc = ble_sm_inject_io(event->passkey.conn_handle, &pkey);
            ESP_LOGI(tag, "ble_sm_inject_io result: %d", rc);
        } else if (event->passkey.params.action == BLE_SM_IOACT_NUMCMP) {
            ESP_LOGI(tag, "Passkey on device's display: %" PRIu32 , event->passkey.params.numcmp);
            ESP_LOGI(tag, "Accept or reject the passkey through console in this format -> key Y or key N");
            pkey.action = event->passkey.params.action;
            if (scli_receive_key(&key)) {
                pkey.numcmp_accept = key;
            } else {
                pkey.numcmp_accept = 0;
                ESP_LOGE(tag, "Timeout! Rejecting the key");
            }
            rc = ble_sm_inject_io(event->passkey.conn_handle, &pkey);
            ESP_LOGI(tag, "ble_sm_inject_io result: %d", rc);
        } else if (event->passkey.params.action == BLE_SM_IOACT_OOB) {
            static uint8_t tem_oob[16] = {0};
            pkey.action = event->passkey.params.action;
            for (int i = 0; i < 16; i++) {
                pkey.oob[i] = tem_oob[i];
            }
            rc = ble_sm_inject_io(event->passkey.conn_handle, &pkey);
            ESP_LOGI(tag, "ble_sm_inject_io result: %d", rc);
        } else if (event->passkey.params.action == BLE_SM_IOACT_INPUT) {
            ESP_LOGI(tag, "Enter the passkey through console in this format-> key 123456");
            pkey.action = event->passkey.params.action;
            if (scli_receive_key(&key)) {
                pkey.passkey = key;
            } else {
                pkey.passkey = 0;
                ESP_LOGE(tag, "Timeout! Passing 0 as the key");
            }
            rc = ble_sm_inject_io(event->passkey.conn_handle, &pkey);
            ESP_LOGI(tag, "ble_sm_inject_io result: %d", rc);
        }
        return 0;

    case BLE_GAP_EVENT_AUTHORIZE:
        MODLOG_DFLT(INFO, "authorize event: conn_handle=%d attr_handle=%d is_read=%d",
                    event->authorize.conn_handle,
                    event->authorize.attr_handle,
                    event->authorize.is_read);

        /* The default behaviour for the event is to reject authorize request */
        event->authorize.out_response = BLE_GAP_AUTHORIZE_REJECT;
        return 0;

#if MYNEWT_VAL(BLE_POWER_CONTROL)
    case BLE_GAP_EVENT_TRANSMIT_POWER:
        MODLOG_DFLT(INFO, "Transmit power event : status=%d conn_handle=%d reason=%d "
                           "phy=%d power_level=%x power_level_flag=%d delta=%d",
                     event->transmit_power.status,
                     event->transmit_power.conn_handle,
                     event->transmit_power.reason,
                     event->transmit_power.phy,
                     event->transmit_power.transmit_power_level,
                     event->transmit_power.transmit_power_level_flag,
                     event->transmit_power.delta);
        return 0;

    case BLE_GAP_EVENT_PATHLOSS_THRESHOLD:
        MODLOG_DFLT(INFO, "Pathloss threshold event : conn_handle=%d current path loss=%d "
                           "zone_entered =%d",
                     event->pathloss_threshold.conn_handle,
                     event->pathloss_threshold.current_path_loss,
                     event->pathloss_threshold.zone_entered);
        return 0;
#endif

#if MYNEWT_VAL(BLE_EATT_CHAN_NUM) > 0
    case BLE_GAP_EVENT_EATT:
        MODLOG_DFLT(INFO, "EATT %s : conn_handle=%d cid=%d",
                event->eatt.status ? "disconnected" : "connected",
                event->eatt.conn_handle,
                event->eatt.cid);
	if (event->eatt.status) {
		/* Abort if disconnected */
		return 0;
	}
	cids[bearers] = event->eatt.cid;
	bearers += 1;
	if (bearers != MYNEWT_VAL(BLE_EATT_CHAN_NUM)) {
		/* Wait until all EATT bearers are connected before proceeding */
		return 0;
	}
	/* Set the default bearer to use for further procedures */
	rc = ble_att_set_default_bearer_using_cid(event->eatt.conn_handle, cids[0]);
	if (rc != 0) {
		MODLOG_DFLT(INFO, "Cannot set default EATT bearer, rc = %d\n", rc);
		return rc;
	}

	return 0;
#endif

#if MYNEWT_VAL(BLE_CONN_SUBRATING)
    case BLE_GAP_EVENT_SUBRATE_CHANGE:
        MODLOG_DFLT(INFO, "Subrate change event : conn_handle=%d status=%d factor=%d",
                    event->subrate_change.conn_handle,
                    event->subrate_change.status,
                    event->subrate_change.subrate_factor);
        return 0;
#endif

    }
    return 0;
}

static void
bleprph_on_reset(int reason)
{
    MODLOG_DFLT(ERROR, "Resetting state; reason=%d\n", reason);
}

#if CONFIG_EXAMPLE_RANDOM_ADDR
static void
ble_app_set_addr(void)
{
    ble_addr_t addr;
    int rc;

    /* generate new non-resolvable private address */
    rc = ble_hs_id_gen_rnd(0, &addr);
    assert(rc == 0);

    /* set generated address */
    rc = ble_hs_id_set_rnd(addr.val);

    assert(rc == 0);
}
#endif

static void
bleprph_on_sync(void)
{
    int rc;


    rc = ble_hs_util_ensure_addr(0);

    /* Figure out address to use while advertising (no privacy for now) */
    rc = ble_hs_id_infer_auto(0, &own_addr_type);
    if (rc != 0) {
        MODLOG_DFLT(ERROR, "error determining address type; rc=%d\n", rc);
        return;
    }

    /* Printing ADDR */
    uint8_t addr_val[6] = {0};
    rc = ble_hs_id_copy_addr(own_addr_type, addr_val, NULL);

    MODLOG_DFLT(INFO, "Device Address: ");
    print_addr(addr_val);
    MODLOG_DFLT(INFO, "\n");
    /* Begin advertising. */
    bleprph_advertise();

}

void bleprph_host_task(void *param)
{
    ESP_LOGI(tag, "BLE Host Task Started");
    /* This function will return only when nimble_port_stop() is executed */
    nimble_port_run();

    nimble_port_freertos_deinit();
}

void
Bluetooth_task(void)
{
    int rc;

    /* Initialize NVS — it is used to store PHY calibration data */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ret = nimble_port_init();
    if (ret != ESP_OK) {
        ESP_LOGE(tag, "Failed to init nimble %d ", ret);
        return;
    }
    /* Initialize the NimBLE host configuration. */
    ble_hs_cfg.reset_cb = bleprph_on_reset;
    ble_hs_cfg.sync_cb = bleprph_on_sync;
    ble_hs_cfg.gatts_register_cb = gatt_svr_register_cb;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    ble_hs_cfg.sm_io_cap = CONFIG_EXAMPLE_IO_TYPE;
#ifdef CONFIG_EXAMPLE_BONDING
    ble_hs_cfg.sm_bonding = 1;
    /* Enable the appropriate bit masks to make sure the keys
     * that are needed are exchanged
     */
    ble_hs_cfg.sm_our_key_dist |= BLE_SM_PAIR_KEY_DIST_ENC;
    ble_hs_cfg.sm_their_key_dist |= BLE_SM_PAIR_KEY_DIST_ENC;
#endif


//make sure clients allow secure connections
    ble_hs_cfg.sm_sc = 1;

    rc = gatt_svr_init();
    assert(rc == 0);

    /* Set the default device name. */
    rc = ble_svc_gap_device_name_set("ESP32GPS");
    assert(rc == 0);

    /* XXX Need to have template for store */
    ble_store_config_init();

    nimble_port_freertos_init(bleprph_host_task);

}
