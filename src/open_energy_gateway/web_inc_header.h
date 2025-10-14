//Raw string literal quotation, webpage HTML header...
String web_inc_header = R"(
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
    /* Universal table styles - enhanced from existing */
    table {
      border-collapse: collapse; 
      width: 100%;
      max-width: 1400px;
      margin: 10px 0;
    }
    table th {
      border: 1px solid black; 
      padding: 12px; 
      background-color: #cccccc;
      text-align: left;
      font-weight: bold;
      color: #333;
    }
    table td {
      border: 1px solid black; 
      padding: 12px;  
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
    
   /* Container for meter list and other containers */
   .container-div {
      background: white;
      border-radius: 8px;
      padding: 20px;
      margin: 20px 0;
      box-shadow: 0 2px 4px rgba(0,0,0,0.1);
    }
  </style>
</head>
<body>
<main>
  )";