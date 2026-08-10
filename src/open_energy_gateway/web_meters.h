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
    <p>Connection status: <span id="connection_status" style="color: gray;">Connecting...</span></p>
    <p>Last update: <span id="last_update">--:--:--</span></p>
    <br>
    <p>Values updates every 5 seconds. Initial connection typically takes up to 30 seconds.</p>  
  </div>


  <div class="card">
    <h2>Select Meter</h2>
        <label for="meter_selector" style="display: block; margin-bottom: 8px; font-weight: 500;">Select Meter:</label>
        <select id="meter_selector" style="padding: 8px 12px; font-size: 16px; border: 1px solid #ccc; border-radius: 4px; min-width: 150px;">
          <option value="1" id="meter_selector_option_1">Meter 1</option>
          <option value="2" id="meter_selector_option_2">Meter 2</option>
          <option value="3" id="meter_selector_option_3">Meter 3</option>
          <option value="4" id="meter_selector_option_4">Meter 4</option>
          <option value="5" id="meter_selector_option_5">Meter 5</option>
          <option value="6" id="meter_selector_option_6">Meter 6</option>
          <option value="7" id="meter_selector_option_7">Meter 7</option>
          <option value="8" id="meter_selector_option_8">Meter 8</option>
          <option value="9" id="meter_selector_option_9">Meter 9</option>
          <option value="10" id="meter_selector_option_10">Meter 10</option>
        </select>
  </div>
  
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
        setTimeout(connectToWebSocket, 1000); //1 seconds delay between reconnection attempts
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

// Meter selector functionality
document.addEventListener('DOMContentLoaded', function() {
  
  var maxNumberOfMeters = 10; // Maximum number of meters supported
  //value will be replaced by the server
  var numberOfMeters = numberOfMetersValue; // Actual number of connected meters
  
  const meterSelector = document.getElementById('meter_selector');
  const urlParams = new URLSearchParams(window.location.search);
  const currentMeter = urlParams.get('id') || '1';
  
  // Hide options beyond numberOfMeters (following same pattern as home/settings pages)
  for (let i = 1; i <= maxNumberOfMeters; i++) {
    const option = document.getElementById('meter_selector_option_' + i);
    if (option) {
      if (i > numberOfMeters) {
        option.style.display = 'none';
      } else {
        option.style.display = '';
      }
    }
  }
  
  // Set the dropdown to the current meter
  meterSelector.value = currentMeter;
  
  // Handle meter selection change
  meterSelector.addEventListener('change', function() {
    const selectedMeter = this.value;
    window.location.href = '/meters?id=' + selectedMeter;
  });
});

</script>
</html>


)";