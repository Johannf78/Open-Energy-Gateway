//Raw string literal quotation, webpage HTML...
String webpage_settings = R"(

  <h1>AmpX Open Energy Gateway - Settings</h1>
  <br>
  <h2>Information/Status</h2>
  <ul>
    <li>Gateway ID: <span id='m_gateway_id'>Loading...</span></li>
    <li>Wi-Fi Strength: <span id='m_wifi_rssi'>Loading...</span></li>
    <li>Connected Meters: <span id='m_connected_meters_num'>Loading...</span></li>
  </ul>
  <h2>Set Meter Names</h2>
  <p>To identify meters, set a name for each meter.</p>
  
  <form action='update_meters_name' method='post' enctype='application/x-www-form-urlencoded'>
    <div id='meter1'>
      <h3>Meter: 1, Serial number: <span id='m1_serial_number'>Loading...</span></h3>
      <label for='m1_name'>Meter name: </label><input type='text' id='m1_name' name='m1_name' value=''>
    </div>

    <div id='meter2'>
      <h3>Meter: 2, Serial number: <span id='m2_serial_number'>Loading...</span></h3>
      <label for='m2_name'>Meter name: </label><input type='text' id='m2_name' name='m2_name' value=''>
    </div>

    <div id='meter3'>
      <h3>Meter: 3, Serial number: <span id='m3_serial_number'>Loading...</span></h3>
      <label for='m3_name'>Meter name: </label><input type='text' id='m3_name' name='m3_name' value=''>
    </div>

    <div id='meter4'>
      <h3>Meter: 4, Serial number: <span id='m4_serial_number'>Loading...</span></h3>
      <label for='m4_name'>Meter name: </label><input type='text' id='m4_name' name='m4_name' value=''>
    </div>

    <br>
    <input type='submit' value='update'>
  </form>

  <br>
  <h2>Meter Firmware Update</h2>
  <form action='update' method='post'>
    <p>
    <label>Click the button to download and update the device firmware:</label>
    <input type='submit' value='update'></p>
  </form>

  <p>Connection status: <span id="connection_status" style="color: gray;">Connecting...</span></p>
  <p>Last update: <span id="last_update">--:--:--</span></p>

  <br>
  <p><a href="/">Home </a></p>

</main>
<footer>
<br/><br/>
<span><a href='https://ampx.co/' target='_blank'>https://ampx.co</a></span>
</footer>
</body>
<script type="text/javascript">
  
  var socket;
  var reconnectAttempts = 0;
  var maxReconnectAttempts = 10;
  var initialDataReceived = false;  // Flag to track if initial data has been received

  function init(){
    //This variable is updated by the Arduino code before sending the HTML
    var numberOfMeters = numberOfMetersValue; // Define the number of meters as an integer
    //document.getElementById(key).value = value;

    // Hide the div with id 'meter2', 'meter3', and 'meter4' based on the numberOfMeters
    if (numberOfMeters == 1) {
        document.getElementById('meter2').style.display = 'none';
        document.getElementById('meter3').style.display = 'none';
        document.getElementById('meter4').style.display = 'none';
    } else if (numberOfMeters == 2) {
        document.getElementById('meter3').style.display = 'none';
        document.getElementById('meter4').style.display = 'none';
    } else if (numberOfMeters == 3) {
        document.getElementById('meter4').style.display = 'none';
    }
 
    connectToWebSocket();
  }

  function connectToWebSocket(){
    socket = new WebSocket('ws://' + window.location.hostname + ':81/');
    // Reset the initial data flag when reconnecting
    initialDataReceived = false;
    
    socket.onopen = function() {
      console.log('WebSocket connected');
      reconnectAttempts = 0; // Reset counter on successful connection

      document.getElementById('connection_status').textContent = 'Connected';
      document.getElementById('connection_status').style.color = 'green';
    };

    socket.onmessage = function(event) {
      processCommand(event);
    };

    socket.onerror = function(error) {
      console.log('WebSocket error');
      document.getElementById('connection_status').textContent = 'Error';
      document.getElementById('connection_status').style.color = 'red';
    };

    socket.onclose = function(event) {
      console.log('WebSocket closed');

      document.getElementById('connection_status').textContent = 'Disconnected';
      document.getElementById('connection_status').style.color = 'red';

      if (reconnectAttempts < maxReconnectAttempts) {
        reconnectAttempts++;
        console.log('Reconnecting... attempt ' + reconnectAttempts);
        setTimeout(connectToWebSocket, 5000);
      } else {
        console.log('Max reconnection attempts reached');
      }
    };
  }


  function processCommand(event){
    var data = JSON.parse(event.data);
    if (data) {
      for (let key in data) {
        let value = data[key];
        
        // Handle different types of data
        if (key === 'm_wifi_rssi') {
          document.getElementById('m_wifi_rssi').textContent = value;
        } else if (key === 'm_gateway_id') {
          document.getElementById('m_gateway_id').textContent = value;
        } else if (key === 'm_connected_meters_num') {
          document.getElementById('m_connected_meters_num').textContent = value;
        } else if (key.includes('serial_number')) {
          document.getElementById(key).textContent = value;
        } else if (!initialDataReceived && key.includes('name')) 
        {
          // Update the input field value for meter names
          //only update the input field value for meter names the first time the page is loaded
          let inputElement = document.getElementById(key);
          if (inputElement) {
            inputElement.value = value;
          }
        }
      }

        // Mark that initial data has been received after first WebSocket message
      initialDataReceived = true;
    }

    // Update the last update time
    var now = new Date();
    document.getElementById('last_update').textContent = now.toLocaleTimeString();

  }

  window.onload = function(event){
    init();
  }
</script>
</html>
  )";
