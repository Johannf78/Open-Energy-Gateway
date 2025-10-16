//Raw string literal quotation, webpage HTML...

String webpage_meters = R"(
<main class="main-content">
  
  <!-- Header -->
  <header class="header">
    <h1>Meter Details</h1>
  </header>

  <!-- Content -->
  <div class="content">

  <div class="card">
    <h2>Meter: <span id="meter_number">METER_NUMBER</span></h2>
    <p><strong>Serial Number:</strong> <span id='meter_serial_number'></span></p>
    <p><strong>Meter Name:</strong> <span id='meter_name'></span></p>
  </div>

  <div class="card">
    
  <table>
    <tr>
      <th>Entity</th>
      <th>L1</th>
      <th>L2</th>
      <th>L3</th>
      <th>Avg/Max</th>
    </tr>
    <tr>
      <td class='text-align-left'>Voltage</td>
      <td id='meter_voltage_L1'>0.00 V</td>
      <td id='meter_voltage_L2'>0.00 V</td>
      <td id='meter_voltage_L3'>0.00 V</td>
      <td></td>
    </tr>
    <tr>
      <td class='text-align-left'>Current</td>
      <td id='meter_current_L1'>0.00 A</td>
      <td id='meter_current_L2'>0.00 A</td>
      <td id='meter_current_L3'>0.00 A</td>
      <td id='meter_current_avg'>0.00 A</td>
    </tr>
    <tr>
      <td class='text-align-left'>Active Power</td>
      <td id='meter_active_power_L1'>0.00 KW</td>
      <td id='meter_active_power_L2'>0.00 KW</td>
      <td id='meter_active_power_L3'>0.00 KW</td>
      <td id='meter_active_power_tot'>0.00 KW</td>
    </tr>
    <tr>
      <td class='text-align-left'>Active Energy Imported</td>
      <td id='meter_active_energy_imported_L1'>0 kWh</td>
      <td id='meter_active_energy_imported_L2'>0 kWh</td>
      <td id='meter_active_energy_imported_L3'>0 kWh</td>
      <td id='meter_active_energy_imported_tot'>0 kWh</td>
    </tr>
    </table>
  </div>
  
  <p>Connection status: <span id="connection_status" style="color: gray;">Connecting...</span></p>
  <p>Last update: <span id="last_update">--:--:--</span></p>
  
  <p><a href="/">← Back to Home</a></p>

    </div> <!-- End of content -->
  </main>
</div> <!-- End of app-container -->

<footer>
<br/><br/>
<span><a href='https://ampx.co/' target='_blank'>https://ampx.co</a></span>
</footer>
</body>

<script type="text/javascript">

  var socket;
  var reconnectAttempts = 0;
  var maxReconnectAttempts = 10;
  var meterNumber = METER_NUMBER; // Will be replaced by server

  function init(){
    connectToWebSocket();
  }

  function connectToWebSocket(){
    socket = new WebSocket('ws://' + window.location.hostname + ':81/');
    
    socket.onopen = function() {
      console.log('WebSocket connected');
      reconnectAttempts = 0;
      document.getElementById('connection_status').textContent = 'Connected';
      document.getElementById('connection_status').style.color = 'green';
    };

    socket.onmessage = function(event) {
      console.log('WebSocket message received');
      processCommand(event);

      var now = new Date();
      document.getElementById('last_update').textContent = now.toLocaleTimeString();
    };

    socket.onerror = function(error) {
      console.log('WebSocket error');
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
      // Build the meter prefix
      var meterPrefix = 'm' + meterNumber + '_';
      
      for (let key in data) {
        // Only process keys for this specific meter
        if (key.startsWith(meterPrefix)) {
          let value = data[key];
          
          // Remove the meter prefix to get the generic key
          let genericKey = key.replace(meterPrefix, 'meter_');

          if (key.includes('voltage'))
            value += ' V';
          else if (key.includes('current'))
            value += ' A';
          else if (key.includes('power'))
            value += ' kW';
          else if (key.includes('energy'))
            value += ' kWh';

          let element = document.getElementById(genericKey);
          if (element) {
            element.textContent = value;
          }
        }
      }
    }
  }

  window.onload = function(event){
    init();
  }
</script>
</html>


)";