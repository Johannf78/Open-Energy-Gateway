//Raw string literal quotation, webpage HTML header...
String web_inc_header = R"(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name='viewport' content='width=device-width, initial-scale=1'>
  <title>AmpX Open Energy Gateway</title>
  <style>
    /*Reset all styles*/
    * {
      margin: 0;
      padding: 0;
      box-sizing: border-box;
    }
      
    body { 
      font-family: Arial, sans-serif;  
      background-color: #EEEEEE;  
      -webkit-font-smoothing: antialiased;
      -moz-osx-font-smoothing: grayscale;     
    }
    .app-container {
      display: flex;
      min-height: 100vh;
      background-color: #f1f2f4;
    }

      /* Sidebar Styles */
    .sidebar {
      width: 220px;
      background-color: #374151;
      border-right: 1px solid #1f2937;
      flex-shrink: 0;
    }

    .sidebar-header {
      padding: 24px;
      border-bottom: 1px solid #4b5563;
    }

    .sidebar-header h2 {
      font-size: 18px;
      font-weight: 600;
      color: #ffffff;
    }

    .sidebar-nav {
      padding: 16px 12px;
    }

    .sidebar-nav ul {
      list-style: none;
    }

    .sidebar-nav li {
      margin-bottom: 4px;
    }

    .sidebar-nav a {
      display: flex;
      align-items: center;
      gap: 12px;
      padding: 10px 12px;
      text-decoration: none;
      color: #ffffff;
      border-radius: 8px;
      transition: background-color 0.2s;
    }

    .sidebar-nav a:hover {
      background-color: #4b5563;
    }

    .sidebar-nav a.active {
      background-color: #4b5563;
      color: #ffffff;
      font-weight: 500;
    }

    .sidebar-nav svg {
      width: 20px;
      height: 20px;
    }

    /* Main Content Styles */
    .main-content {
      flex: 1;
      display: flex;
      flex-direction: column;
    }

    .header {
      background-color: #ffffff;
      border-bottom: 1px solid #e5e7eb;
      padding: 16px 24px;
    }

    .header h1 {
      font-size: 24px;
      font-weight: 600;
      color: #111827;
    }

    .content {
      padding: 24px;
      max-width: 1280px;
      margin: 0 auto;
      width: 100%;
    }
  

    footer { 
      margin: auto;   
      border: 0px solid black;  
      padding: 20px; 
    }


   /* Container for meter list and other containers */
   .card {
      background: white;
      border-radius: 8px;
      padding: 20px;
      margin: 20px 0;
      box-shadow: 0 2px 4px rgba(0,0,0,0.1);
    }

    
    /*span { color: #003366; }*/
    
    /* Universal table styles - enhanced from existing */
    table {
      border-collapse: collapse; 
      width: 100%;
      max-width: 700px;
      margin: 10px 0;
    }
    table th {
      border: 1px solid black; 
      padding: 8px; 
      background-color: #cccccc;
      text-align: center;
      font-weight: bold;
      color: #333;
    }
    table td {
      border: 1px solid black; 
      padding: 8px;  
      text-align: right;
      color: #333;
    }
    table tbody tr:hover {
      background-color: #f9f9f9;
    }

    .text-align-left {text-align: left;}

    /* Button/Link styles */
    .view-button {
      background-color: #e0e0e0;
      color: #333;
      text-decoration: none;
      padding: 8px 16px;
      border-radius: 4px;
      font-size: 14px;
      display: inline-block;
    }
    .view-button:hover {
      background-color: #d0d0d0;
    }
    
    .card ul {
      margin-left: 20px;
      padding-left: 20px;
    }

</style>
</head>
<body>
<div class="app-container">
    <!-- Sidebar -->
    <aside class="sidebar">
    <div class="sidebar-header">
      <h2>AmpX Open Energy Gateway</h2>
    </div>
    <nav class="sidebar-nav">
      <ul>
        <li>
          <a href="/" >
              <svg
                xmlns="http://www.w3.org/2000/svg"
                width="20"
                height="20"
                viewBox="0 0 24 24"
                fill="none"
                stroke="currentColor"
                stroke-width="2"
                stroke-linecap="round"
                stroke-linejoin="round"
              >
              <path d="m3 9 9-7 9 7v11a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2z" />
              <polyline points="9 22 9 12 15 12 15 22" />
            </svg>
            <span>Home</span>
          </a>
        </li>
        <li>
          <a href="/meters">
                <svg
                  xmlns="http://www.w3.org/2000/svg"
                  width="20"
                  height="20"
                  viewBox="0 0 24 24"
                  fill="none"
                  stroke="currentColor"
                  stroke-width="2"
                  stroke-linecap="round"
                  stroke-linejoin="round"
                >
              <path d="m12 14 4-4" />
              <path d="M3.34 19a10 10 0 1 1 17.32 0" />
            </svg>
            <span>Meters</span>
          </a>
        </li>
        <li>
          <a href="/settings">
                <svg
                  xmlns="http://www.w3.org/2000/svg"
                  width="20"
                  height="20"
                  viewBox="0 0 24 24"
                  fill="none"
                  stroke="currentColor"
                  stroke-width="2"
                  stroke-linecap="round"
                  stroke-linejoin="round"
                >
              <path d="M12.22 2h-.44a2 2 0 0 0-2 2v.18a2 2 0 0 1-1 1.73l-.43.25a2 2 0 0 1-2 0l-.15-.08a2 2 0 0 0-2.73.73l-.22.38a2 2 0 0 0 .73 2.73l.15.1a2 2 0 0 1 1 1.72v.51a2 2 0 0 1-1 1.74l-.15.09a2 2 0 0 0-.73 2.73l.22.38a2 2 0 0 0 2.73.73l.15-.08a2 2 0 0 1 2 0l.43.25a2 2 0 0 1 1 1.73V20a2 2 0 0 0 2 2h.44a2 2 0 0 0 2-2v-.18a2 2 0 0 1 1-1.73l.43-.25a2 2 0 0 1 2 0l.15.08a2 2 0 0 0 2.73-.73l.22-.39a2 2 0 0 0-.73-2.73l-.15-.08a2 2 0 0 1-1-1.74v-.5a2 2 0 0 1 1-1.74l.15-.09a2 2 0 0 0 .73-2.73l-.22-.38a2 2 0 0 0-2.73-.73l-.15.08a2 2 0 0 1-2 0l-.43-.25a2 2 0 0 1-1-1.73V4a2 2 0 0 0-2-2z" />
              <circle cx="12" cy="12" r="3" />
            </svg>
            <span>Settings</span>
          </a>
        </li>
      </ul>
    </nav>
  </aside>

  <script>
  // Auto-highlight active navigation based on current page
  function setActiveNav() {
    const currentPath = window.location.pathname;
    const navLinks = document.querySelectorAll('.sidebar-nav a');
    
    navLinks.forEach(link => {
      link.classList.remove('active');
      
      // Get the href attribute and normalize it
      const linkPath = link.getAttribute('href');
      
      // Check for exact match or if current path starts with link path (for /meters?id=N)
      if (linkPath === currentPath || 
          (linkPath !== '/' && currentPath.startsWith(linkPath))) {
        link.classList.add('active');
      }
      // Special case: highlight Home only on exact root match
      else if (linkPath === '/' && currentPath === '/') {
        link.classList.add('active');
      }
    });
  }
  
  // Run on page load
  document.addEventListener('DOMContentLoaded', setActiveNav);
</script>

  )";