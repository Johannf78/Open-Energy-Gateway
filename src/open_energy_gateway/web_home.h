//Raw string literal quotation, webpage HTML...

String webpage_home = R"(
<!DOCTYPE html>
<html>
<head>
  <title>AmpX Open Energy Gateway</title>
  <meta name='viewport' content='width=device-width, initial-scale=1'>
  <style>
    body { background-color: #EEEEEE; font-family: Arial, sans-serif; }
    main { margin: auto;  border: 3px solid black;  padding: 20px; }
    footer { margin: auto;   border: 0px solid black;  padding: 20px; }
    span { color: #003366; }
    table {border-collapse: collapse; }
    table th {border: 1px solid black; padding: 5px; background-color: #cccccc;}
    table td {border: 1px solid black; padding: 5px;  text-align: right;}
    .text-align-left {text-align: left;}
  </style>
</head>
<body>
<main>
  <h1>AmpX Open Energy Gateway</h1>

  <h2>Energy Consumption Summary</h2>
  <table>
  <tr>
    <th>Meter</th>
    <th>Total active energy imported</th>
  </tr>
  <tr id='meter1_summary'>
    <td>1</td>
    <td id='m1_active_energy_imported_tot_summary'>0 kWh</td>
  </tr>
  <tr id='meter2_summary'>
    <td>2</td>
    <td id='m2_active_energy_imported_tot_summary'>0 kWh</td>
  </tr>
  <tr id='meter3_summary'>
    <td>3</td>
    <td id='m3_active_energy_imported_tot_summary'>0 kWh</td>
  </tr>
  <tr id='meter4_summary'>
    <td>4</td>
    <td id='m4_active_energy_imported_tot_summary'>0 kWh</td>
  </tr>
  </table>
  



  <br/>
  <div id='meter1'>
    <h3>Meter: 1, Serial number: <span id='m1_serial_number'></span>, Meter name: <span id='m1_name'></span></h3>
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
      <td id='m1_voltage_L1'>0.00 V</td>
      <td id='m1_voltage_L2'>0.00 V</td>
      <td id='m1_voltage_L3'>0.00 V</td>
      <td></td>
    </tr>
    <tr>
      <td class='text-align-left'>Current</td>
      <td id='m1_current_L1'>0.00 A</td>
      <td id='m1_current_L2'>0.00 A</td>
      <td id='m1_current_L3'>0.00 A</td>
      <td id='m1_current_avg'>0.00 A</td>
    </tr>
    <tr>
      <td class='text-align-left'>Active Power</td>
      <td id='m1_active_power_L1'>0.00 KW</td>
      <td id='m1_active_power_L2'>0.00 KW</td>
      <td id='m1_active_power_L3'>0.00 KW</td>
      <td id='m1_active_power_tot'>0.00 KW</td>
    </tr>
    <tr>
      <td class='text-align-left'>Active Energy Imported</td>
      <td id='m1_active_energy_imported_L1'>0 kWh</td>
      <td id='m1_active_energy_imported_L2'>0 kWh</td>
      <td id='m1_active_energy_imported_L3'>0 kWh</td>
      <td id='m1_active_energy_imported_tot'>0 kWh</td>
    </tr>
    </table>
  </div>

  <br/>
  <div id='meter2'>
    <h3>Meter: 2, Serial number: <span id='m2_serial_number'></span>, Meter name: <span id='m2_name'></span></h3>
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
      <td id='m2_voltage_L1'>0.00 V</td>
      <td id='m2_voltage_L2'>0.00 V</td>
      <td id='m2_voltage_L3'>0.00 V</td>
      <td></td>
    </tr>
    <tr>
      <td class='text-align-left'>Current</td>
      <td id='m2_current_L1'>0.00 A</td>
      <td id='m2_current_L2'>0.00 A</td>
      <td id='m2_current_L3'>0.00 A</td>
      <td id='m2_current_avg'>0.00 A</td>
    </tr>
    <tr>
      <td class='text-align-left'>Active Power</td>
      <td id='m2_active_power_L1'>0.00 KW</td>
      <td id='m2_active_power_L2'>0.00 KW</td>
      <td id='m2_active_power_L3'>0.00 KW</td>
      <td id='m2_active_power_tot'>0.00 KW</td>
    </tr>
    <tr>
      <td class='text-align-left'>Active Energy Imported</td>
      <td id='m2_active_energy_imported_L1'>0 kWh</td>
      <td id='m2_active_energy_imported_L2'>0 kWh</td>
      <td id='m2_active_energy_imported_L3'>0 kWh</td>
      <td id='m2_active_energy_imported_tot'>0 kWh</td>
    </tr>
    </table>
  </div>

  <br/>
  <div id='meter3'>
    <h3>Meter: 3, Serial number: <span id='m3_serial_number'></span>, Meter name: <span id='m3_name'></span></h3>
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
      <td id='m3_voltage_L1'>0.00 V</td>
      <td id='m3_voltage_L2'>0.00 V</td>
      <td id='m3_voltage_L3'>0.00 V</td>
      <td></td>
    </tr>
    <tr>
      <td class='text-align-left'>Current</td>
      <td id='m3_current_L1'>0.00 A</td>
      <td id='m3_current_L2'>0.00 A</td>
      <td id='m3_current_L3'>0.00 A</td>
      <td id='m3_current_avg'>0.00 A</td>
    </tr>
    <tr>
      <td class='text-align-left'>Active Power</td>
      <td id='m3_active_power_L1'>0.00 KW</td>
      <td id='m3_active_power_L2'>0.00 KW</td>
      <td id='m3_active_power_L3'>0.00 KW</td>
      <td id='m3_active_power_tot'>0.00 KW</td>
    </tr>
    <tr>
      <td class='text-align-left'>Active Energy Imported</td>
      <td id='m3_active_energy_imported_L1'>0 kWh</td>
      <td id='m3_active_energy_imported_L2'>0 kWh</td>
      <td id='m3_active_energy_imported_L3'>0 kWh</td>
      <td id='m3_active_energy_imported_tot'>0 kWh</td>
    </tr>
    </table>
  </div>


  <br/>
  <div id='meter4'>
    <h3>Meter: 4, Serial number: <span id='m4_serial_number'></span>, Meter name: <span id='m4_name'></span></h3>
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
      <td id='m4_voltage_L1'>0.00 V</td>
      <td id='m4_voltage_L2'>0.00 V</td>
      <td id='m4_voltage_L3'>0.00 V</td>
      <td></td>
    </tr>
    <tr>
      <td class='text-align-left'>Current</td>
      <td id='m4_current_L1'>0.00 A</td>
      <td id='m4_current_L2'>0.00 A</td>
      <td id='m4_current_L3'>0.00 A</td>
      <td id='m4_current_avg'>0.00 A</td>
    </tr>
    <tr>
      <td class='text-align-left'>Active Power</td>
      <td id='m4_active_power_L1'>0.00 KW</td>
      <td id='m4_active_power_L2'>0.00 KW</td>
      <td id='m4_active_power_L3'>0.00 KW</td>
      <td id='m4_active_power_tot'>0.00 KW</td>
    </tr>
    <tr>
      <td class='text-align-left'>Active Energy Imported</td>
      <td id='m4_active_energy_imported_L1'>0 kWh</td>
      <td id='m4_active_energy_imported_L2'>0 kWh</td>
      <td id='m4_active_energy_imported_L3'>0 kWh</td>
      <td id='m4_active_energy_imported_tot'>0 kWh</td>
    </tr>
    </table>
  </div>

<p>Connection status: <span id="connection_status" style="color: gray;">Connecting...</span></p>
<p>Last update: <span id="last_update">--:--:--</span></p>

<p>Values updates every 3 seconds.</p>
<p><a href="/settings">Settings </a></p>


</main>
<footer>
<br/><br/>
<span><a href='https://ampx.co/' target='_blank'>https://ampx.co</a></span>
</footer>
</body>
<script type="text/javascript">

  // Initialize the socket variable to be used in the connectToWebSocket function
  var socket;
  var reconnectAttempts = 0;
  var maxReconnectAttempts = 10;

  function init(){
    //This variable is updated by the Arduino code before sending the HTML
    var numberOfMeters = numberOfMetersValue; // Define the number of meters as an integer
  

      // Hide the div with id 'meter2', 'meter3', and 'meter4' based on the numberOfMeters
      // Also hide the summary table rows
    if (numberOfMeters == 1) {
        document.getElementById('meter2_summary').style.display = 'none';
        document.getElementById('meter3_summary').style.display = 'none';
        document.getElementById('meter4_summary').style.display = 'none';

        document.getElementById('meter2').style.display = 'none';
        document.getElementById('meter3').style.display = 'none';
        document.getElementById('meter4').style.display = 'none';
    } else if (numberOfMeters == 2) {
        document.getElementById('meter3_summary').style.display = 'none';
        document.getElementById('meter4_summary').style.display = 'none';

        document.getElementById('meter3').style.display = 'none';
        document.getElementById('meter4').style.display = 'none';
    } else if (numberOfMeters == 3) {
        document.getElementById('meter4_summary').style.display = 'none';

        document.getElementById('meter4').style.display = 'none';
    }

    connectToWebSocket();

  }



  function connectToWebSocket(){
  
    socket = new WebSocket('ws://' + window.location.hostname + ':81/');
    
    socket.onopen = function() {
      console.log('WebSocket connected');
      reconnectAttempts = 0; // Reset counter on successful connection
      document.getElementById('connection_status').textContent = 'Connected';
      document.getElementById('connection_status').style.color = 'green';
    };

    socket.onmessage = function(event) {
      //for debugging purposes, log the message to the console
      console.log('WebSocket message received');
      //console.log(event.data);

      processCommand(event);

      //Update the last update time
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

      // Try to reconnect to the WebSocket after 5 seconds
      if (reconnectAttempts < maxReconnectAttempts) {
      reconnectAttempts++;
      console.log('Reconnecting... attempt ' + reconnectAttempts);
      setTimeout(connectToWebSocket, 5000);
    } else {
      console.log('Max reconnection attempts reached');
      // Optionally show error message to user
    }
    };
  };
  

  function processCommand(event){
    var data = JSON.parse(event.data);
    
    if (data) {
      for (let key in data) {
        let value = data[key];

        //for debugging purposes, log the key and value to the console
        //console.log('Key:', key, 'Value:', value);

        if (key.includes('voltage'))
          value += ' V';
        else if (key.includes('current'))
          value += ' A';
        else if (key.includes('power'))
          value += ' kW';
        else if (key.includes('energy'))
          value += ' kWh';
      else if (key.includes('name'))
          value =  value; // Just use the value directly
      // Note: serial numbers don't need units, so no special handling
           
           
        //console.log("Key:", key, "Value:", value);

        //Use the key to find the element and update the innerHTML
        let element = document.getElementById(key);
        if (element) {
          element.textContent = value;
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
