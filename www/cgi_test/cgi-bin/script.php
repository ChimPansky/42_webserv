<?php
$method = $_SERVER['REQUEST_METHOD'];


if ($method === "POST") {
    $buffer = file_get_contents("php://stdin");
} else {
    $buffer = $_SERVER['QUERY_STRING'];

}

$form = [];
parse_str($buffer, $form);

$first_name = isset($form['first_name']) ? htmlspecialchars($form['first_name'], ENT_QUOTES, 'UTF-8') : '';
$last_name = isset($form['last_name']) ? htmlspecialchars($form['last_name'], ENT_QUOTES, 'UTF-8') : '';

echo "Content-type:text/html\n\n";
echo "<html>";
echo "<head>";
echo "<title>Hello - CGI Program</title>";
echo "</head>";
echo "<body>";
echo "<h2>Hello $first_name $last_name</h2>";
echo "<p>Request method: $method</p>";
echo "</body>";
echo "</html>";
?>
