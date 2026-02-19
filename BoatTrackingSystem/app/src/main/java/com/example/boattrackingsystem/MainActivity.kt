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
import androidx.compose.foundation.lazy.items

import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.Image
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxHeight
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.runtime.mutableStateListOf
import androidx.compose.runtime.remember
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.painterResource
import com.example.boattrackingsystem.BleManager
class MainActivity : ComponentActivity() {

    val navyBlueButtons = Color(45, 64, 113);
    val lightBlue = Color( 103,121,164);
    var GpsList = mutableStateListOf<GpsData>()
    private var statusLabel by mutableStateOf("Ready")
    private val bleManager by lazy {
        BleManager(this) { newStatus ->
            statusLabel = newStatus
        }
    }


    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)


        enableEdgeToEdge()
        setContent {
            BoatTrackingSystemTheme() {
                CenterAlignedTopAppBarExample(
                    bleManager,
                    label = statusLabel,
                    onLabelUpdate = { statusLabel = it })

            }
        }
    }


    @Composable
    fun UncleImage() {
        // Make sure to actually put an Image() tag here!
        Image(
            painter = painterResource(id = R.drawable.unclepicture),
            contentDescription = "Uncle",
            modifier = Modifier.fillMaxWidth().height(800.dp)
        )
    }

    @OptIn(ExperimentalMaterial3Api::class)
    @Composable //bleScanner: BleScanner
    fun CenterAlignedTopAppBarExample(
        bleManager: BleManager,
        label: String,
        onLabelUpdate: (String) -> Unit
    ) {
        val scrollBehavior = TopAppBarDefaults.pinnedScrollBehavior(rememberTopAppBarState())

        var isImageVisible by remember { mutableStateOf(false) }
        var displayNumberOfGpsEntries by remember { mutableStateOf(false) }


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
                        IconButton(onClick = {
                            isImageVisible = !isImageVisible
                            displayNumberOfGpsEntries = !displayNumberOfGpsEntries;
                        }) {
                            Icon(
                                imageVector = Icons.Filled.Menu,
                                contentDescription = "Menu"
                            )
                        }
                    },
                    actions = {
                        IconButton(onClick = {
                            bleManager.disconnectDevice()
                            GpsList.clear()
                        }) {

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
                            containerColor= lightBlue,
                            titleContentColor = Color.White,
                            navigationIconContentColor = Color.White,
                            actionIconContentColor = Color.White
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
                    PairWithEsp32Button(
                        bleManager,
                        onLabelChange = { newText -> onLabelUpdate(newText) })
                    Spacer(modifier = Modifier.width(16.dp))

                    ParseGpsMessageButton(
                        bleManager,
                        onLabelChange = { newText -> onLabelUpdate(newText) }
                    )

                }
                if(displayNumberOfGpsEntries)
                {
                    Text("Number of Gps Entries: ${GpsList.size}", modifier = Modifier.padding(16.dp))
                }


                Text(label, modifier = Modifier.padding(16.dp))

                //textbox for  number of
                 var LCmodifier : Modifier = if(isImageVisible)
                 {

                     Modifier.fillMaxHeight(0.60f)
                 }
                else{
                     Modifier

                }

                Column( ){
                LazyColumn(modifier = LCmodifier
                   ) {

                    items(GpsList) { data ->

                        GpsCard(data)
                    }

                }

                if (isImageVisible) {
                    UncleImage()
                }

                }

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
                bleManager.scanForSpecificDevice("ESP32GPS")
            } else {
                onLabelChange("Permissions Denied - Check Settings")
            }
        }

        Button(
             onClick = {
            // Trigger the permissions popup
            permissionLauncher.launch(
                arrayOf(
                    Manifest.permission.BLUETOOTH_SCAN,
                    Manifest.permission.BLUETOOTH_CONNECT
                )
            )
        },                     colors = ButtonDefaults.buttonColors(
                navyBlueButtons,
                contentColor = Color.White
             )
            ) {
            Text("Connect To Esp32")
        }
    }

    @Composable
    fun ParseGpsMessageButton(bleManager: BleManager, onLabelChange: (String) -> Unit) {

        // var stringOfBytes by remember { mutableStateOf<ByteArray?>(null) }
        Button(onClick = {
            val currentbytes = byteArrayOf(
                0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 0x56,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x4E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x45, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x30, 0x31, 0x30, 0x31, 0x32, 0x30,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x4D, 0xCD.toByte(), 0xCC.toByte(),
                0xC7.toByte(), 0x42, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00,
                0x00, 0x20, 0x41, 0x4D, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x20, 0x41, 0x4E
            )
            //stringOfBytes = currentbytes;
            // onLabelChange("Byte array created successfully. Length: $currentbytes.size}")

            //Parse the gps sentence and add it to the Gps Array list
            // Use bleManager to parse the data
            bleManager.ParseGPSSentence(currentbytes)?.let {
                GpsList.add(it)

            }
        },           colors = ButtonDefaults.buttonColors(
            navyBlueButtons,
            contentColor = Color.White
        )) {


            //Text Of Button
            Text("Parse GPS Sentence")
        }
    }


    @Composable
    fun GpsCard(CurrentGpsData: GpsData) {
        Card(
            modifier = Modifier
                .padding(16.dp)
                .fillMaxWidth(),
            shape = RoundedCornerShape(8.dp),
           // elevation = CardDefaults.cardElevation(defaultElevation = 4.dp)
        ) {
            Column(
                modifier = Modifier.padding(16.dp)
            ) {
                Text(text = CurrentGpsData.toString())
            }
        }
    }

}