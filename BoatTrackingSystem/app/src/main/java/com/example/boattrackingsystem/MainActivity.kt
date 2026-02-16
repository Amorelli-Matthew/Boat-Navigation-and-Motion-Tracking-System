package com.example.boattrackingsystem
import android.Manifest
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Menu
import androidx.compose.material3.*
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.input.nestedscroll.nestedScroll
import androidx.compose.ui.res.vectorResource
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import com.example.boattrackingsystem.ui.theme.BoatTrackingSystemTheme


import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.ui.platform.LocalContext

import com.example.boattrackingsystem.BleManager
class MainActivity : ComponentActivity() {

    private var statusLabel by mutableStateOf("Ready")
    private val bleManager by lazy { BleManager(this) { newStatus ->
        statusLabel = newStatus
    }  }



    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        //val adapter = (getSystemService(BLUETOOTH_SERVICE) as BluetoothManager).adapter
       // val bleScanner = BleScanner(adapter)
        enableEdgeToEdge()
        setContent {
            BoatTrackingSystemTheme(){ //bleScanner: BleScanner
                CenterAlignedTopAppBarExample(bleManager, label = statusLabel, onLabelUpdate = { statusLabel = it })

            }
        }
    }
}


@OptIn(ExperimentalMaterial3Api::class)
@Composable //bleScanner: BleScanner
fun CenterAlignedTopAppBarExample(bleManager: BleManager,label: String,onLabelUpdate: (String) -> Unit) {
    val scrollBehavior = TopAppBarDefaults.pinnedScrollBehavior(rememberTopAppBarState())
    Scaffold(
        modifier = Modifier.nestedScroll(scrollBehavior.nestedScrollConnection),
        topBar = {
            CenterAlignedTopAppBar(
                title = {
                    Text(
                        "My App",
                        maxLines = 1,
                        overflow = TextOverflow.Ellipsis
                    )
                },

                navigationIcon = {
                    IconButton(onClick = { onLabelUpdate("Nav button clicked")
                         }) {
                        Icon(
                            imageVector = Icons.Filled.Menu,
                            contentDescription = "Menu"
                        )
                    }
                },
                actions = {
                    IconButton(onClick = {
                        bleManager.disconnectDevice() }) {
                    // onLabelUpdate("Back button clicked") }) {
                        val backIcon =
                            ImageVector.vectorResource(id = R.drawable.arrow_back_24dp_1f1f1f_fill0_wght400_grad0_opsz24)
                        Icon(
                            imageVector = backIcon,
                            contentDescription = "Back"
                        )
                    }
                },
                scrollBehavior = scrollBehavior,
                colors = TopAppBarDefaults.centerAlignedTopAppBarColors(
                    containerColor = MaterialTheme.colorScheme.primaryContainer,
                    titleContentColor = MaterialTheme.colorScheme.primary,
                )
            )
        }
    ) { innerPadding ->
        Column(
            modifier = Modifier
                .padding(innerPadding)
                .fillMaxSize()
        ) {

            Row(
                modifier = Modifier
                    .padding(16.dp),
                ) {
                PairWithEsp32Button( bleManager,     onLabelChange = { newText -> onLabelUpdate(newText) } )
                Spacer(modifier = Modifier.width(16.dp))

                FilledButtonExample(  bleManager,     onLabelChange = { newText -> onLabelUpdate(newText) }



                )
            }



            Text(label, modifier = Modifier.padding(16.dp))
        }
    }
}
@Composable
fun PairWithEsp32Button(bleManager: BleManager, onLabelChange: (String) -> Unit) {
    val context = LocalContext.current

    val permissionLauncher = rememberLauncherForActivityResult(
        ActivityResultContracts.RequestMultiplePermissions()
    ) { permissions ->
        // Check if all permissions were granted
        val allGranted = permissions.values.all { it }

        if (allGranted) {
            onLabelChange("Scanning for ESP32...")
            // Use the method you wrote earlier
            bleManager.scanForSpecificDevice("ESP32GPS")
        } else {
            onLabelChange("Permissions Denied - Check Settings")
        }
    }

    Button(onClick = {
        // Trigger the permissions popup
        permissionLauncher.launch(
            arrayOf(
                Manifest.permission.BLUETOOTH_SCAN,
                Manifest.permission.BLUETOOTH_CONNECT
            )
        )
    }) {
        Text("Connect To Esp32")
    }
}

@Composable
fun FilledButtonExample( bleManager: BleManager, onLabelChange: (String) -> Unit) {
    Button( onClick = { bleManager.RecieveData() }) {
        //Text Of Button
        Text("Parse GPS Sentence")
    }
}
