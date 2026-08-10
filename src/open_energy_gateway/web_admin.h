//Raw string literal quotation, webpage HTML...
String webpage_admin = R"(

<!-- Main Content -->
<main class="main-content">

  <header class="header">
    <h1>Admin</h1>
  </header>

  <div class="content">

  <div class="card">
  <h2>Gateway Reboot</h2><br>
  <form action='/reboot_gateway' method='post' onsubmit="return confirm('Reboot the gateway now?');">
    <input type='submit' value='Reboot Gateway'>
  </form>
  </div>

  <div class="card">
  <h2>Firmware Update (OTA)</h2>
  <p>Current firmware version: <strong id="fw_current">m_firmware_version</strong></p>
  <p>Available version: <strong id="fw_available">m_available_version</strong></p>
  <p>Update status: <strong id="fw_status">m_update_check</strong></p>
  <p>Last OTA result: <strong>m_ota_status</strong></p>
  <p>Last OTA time: <strong>m_ota_time</strong></p>
  <p>Manifest: <code>m_manifest_url</code></p>
  <br>
  <form id="fw_update_form" action='/update' method='post' onsubmit="return confirm('Download and install firmware? Device will reboot.');">
    <input id="fw_update_btn" type='submit' value='Update Firmware' m_update_disabled>
  </form>
  </div>


  <div class="card">
  <h2>Gateway ID Management</h2>
  <form action='/update_gateway_id' method='post' enctype='application/x-www-form-urlencoded'>
    <label for='gateway_id'>Current Gateway ID: </label>
    <span id='current_gateway_id'>m_gateway_id</span>
    <br><br>
    <label for='gateway_id'>Set New Gateway ID: </label>
    <input type='text' id='gateway_id' name='gateway_id' value='m_gateway_id'>
    <br><br>
    <label for='admin_password'>Password: </label>
    <input type='password' id='admin_password' name='admin_password' autocomplete='off'>
    <br><br>
    <input type='submit' value='Update Gateway ID'>
  </form>
  </div>

  
  <p><a href="/">Home</a> | <a href="/settings">Settings</a></p>

  </div>
</main>
</div>
<footer>
<br/><br/>
<span><a href='https://www.ampx.co/' target='_blank'>www.ampx.co</a></span>
</footer>
<script>
  // Poll cached OTA status (loop() fetches version.json; handlers must not use HTTPClient).
  (function () {
    var availableEl = document.getElementById('fw_available');
    var statusEl = document.getElementById('fw_status');
    var btn = document.getElementById('fw_update_btn');
    var attempts = 0;
    function apply(data) {
      if (availableEl) availableEl.textContent = data.available || 'Unavailable';
      if (statusEl) statusEl.textContent = data.status || 'Unknown';
      if (btn) {
        if (data.up_to_date) {
          btn.disabled = true;
        } else {
          btn.disabled = false;
          btn.removeAttribute('disabled');
        }
      }
    }
    function poll() {
      attempts++;
      fetch('/ota_status')
        .then(function (r) { return r.json(); })
        .then(function (data) {
          if (!data.ready && attempts < 20) {
            setTimeout(poll, 1000);
            return;
          }
          apply(data);
        })
        .catch(function () {
          if (attempts < 10) {
            setTimeout(poll, 1000);
            return;
          }
          if (availableEl) availableEl.textContent = 'Unavailable';
          if (statusEl) statusEl.textContent = 'Manifest check failed';
          if (btn) {
            btn.disabled = false;
            btn.removeAttribute('disabled');
          }
        });
    }
    poll();
  })();
</script>
</body>
</html>
  )";
