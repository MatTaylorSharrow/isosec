<?php

function debug($var) {
	echo '<pre>';
	print_r($var);
	echo '</pre>';
}

function err($var) {
	if (is_string($var) || is_numeric($var)) {
		error_log($var);
	} else {
		error_log(print_r($var, true));
	}
}

/**
 * App Controller to manage the processing of the request, generation and 
 * emiting of the response
 */
class App {
	private $server;
	private $get;
	private $post;
	private $env;

	private $allowed_methods = array();
	private $action_map = array();

	private $config = null;

	private $conn = null;

	public function __construct($server, $get, $post, $env) {
		$this->server = $server;
		$this->get = $get;
		$this->post = $post;
		$this->env = $env;
	}

	public function __destruct() {
		// clean up the DB connection;
		$this->cleanup();
	}

	/**
	 * Tell the controller what HTTP methods to allow for REST entities
	 */
	public function setAllowedMethods($methods) {
		// @todo - validation 
		$this->allowed_methods = $methods;
	}

	/**
	 * Set up a simple routing for requests to actions to process the request.
	 */
	public function setActionMap($map) {
		// @todo - validation 
		$this->action_map = $map;
	}

	/**
	 * Start the processing of the request
	 */
	public function begin() {

		// load config
		$this->config = parse_ini_file('../conf/server-app.ini',  true);
err($this->config);

		if ( ! $this->config) {
			echo 'Could not load config file. Aborting';
			return;
		}

		if ( ! $this->connectToDatabase()) {
			$action = new RequestNotAllowedAction();
		}

		$action->process();
		$action->generateResponse();

		$this->cleanup();
	}

	/**
	 * Get the data sent with a request when in json or xml format 
	 *
	 * @return array
	 */
	public function getRawRequestInput() {
		if (trim($this->server['CONTENT_TYPE']) == 'application/json') {
			$json = file_get_contents('php://input');
			$data = json_decode($json);
			return $data;
		}

		// xml etc
		
		return array();
	}


	/**
	 * Clean up any resources
	 */
	private function cleanup() {
		// clean up DB connection
		if (null !== $this->conn) {
			$this->conn->close();
			$this->conn = null;
		}
	}

	/**
	 * Connect to the database using mysqli.  Connection details come from the 
	 * config file.
	 */
	public function connectToDatabase() {
		$conf = $this->config['database'];
		$conn = new mysqli($conf['host'], $conf['database'], $conf['user'], $conf['password']);

		if ($conn->connect_error) {
			return false;
		}

		$this->conn = $conn;
	}

	/**
	 * Obtain a DB connection reference.
	 */
	public function getDbConn() {
		return $this->conn;
	}

	/**
	 * Help method to check for Db connection
	 */
	public function hasDbConnection() {
		return ($this->conn !== null);
	}

	/**
	 * Create an action object depenant on the request input
	 * 
	 * @return Action An Action object for the request
	 */
	private function createAction() {
		if (isset($this->allowed_methods[trim($this->server['REQUEST_URI'])])) {
			$methods = $this->allowed_methods[trim($this->server['REQUEST_URI'])];

			if (in_array($this->server['REQUEST_METHOD'], $methods)) {
				// it's a valid request
				$action_class = $this->action_map[$this->server['REQUEST_URI']];
				$action = new $action_class($this);
				return $action;
			}
		}

		return new RequestNotAllowedAction();
	}
}

/**
 * Simple abstract interface for all objects to process requets
 */
interface Action {
	public function process();
	public function generateResponse();
}


/**
 * Handles processing of the help/usage message action
 */
class ApiUsageAction implements Action {

	public function process() {
		// nothing to do
	}

	/**
	 * Print out the usage / help message
	 */
	public function generateResponse() {
echo '
<html>
    <head>
        <title>Rest API</title>
    </head>
    <body>
		<h1>Rest API</h1>
		<p>This API respects the following entities and methods only.</p>
		<ul>/ - GET (text/html)</ul>
		<ul>
			/AuditLog - POST (APIplication/json), Success 204 response code, Failure 400 response code - with response document
			<pre>
{
	"customer": "string",
	"product": "string", 
	"event_timestamp": "datetime", 
	"device_id": "string"
}
			</pre>
		</ul>
		<ul>All other illegal requests - 405</ul>
    </body>
</html>';

	}
}

/**
 * This action will handle the POSTs to update the Audit Log from client Applications
 */
class RecordAuditLogAction implements Action {

	private $success = false;
	private $app;
	private $errors = array();

	/**
	 * Create Action for updating audit log.  Obtain ref to the App object to enable
	 * access to resources eg config, db, logging etc
	 */
	public function __construct(App $app) {
		$this->app = $app;
	}

	/**
	 * Validate the input data 
	 * 
	 * @return array errors
	 */
	private function validateInput() {
		$errors = array();

		$input = $this->app->getRawRequestInput();
err($input);

		return $errors;
	}

	/**
	 * Process the input request and data
	 *
	 * In this case,  update the AuditLog Table
	 *
	 */
	public function process() {

		// data validation
		$errors = $this->validateInput();

		if (count($errors) > 0) {
			$this->errors = $errors;
			return;
		}


		if ( ! $app->hasDbConnection()) {
			return;
		}

		$conn = $this->app->getDbConn();

		if ($conn) {
			$stmt = $conn->prepare('INSERT INTO AuditLog () VALUES(?, ?, ?, ?, NOW())');

			$stmt->bind_param("s", 's');
			$stmt->bind_param("s", 's');
			$stmt->bind_param("s", 's');
			$stmt->bind_param("s", 's');

			$stmt->execute();

			$this->success = true;
		}
	}

	public function generateResponse() {
		if ( ! $this->success) {
			if ($this->errors) {
			}

			return;
		} 

		//header();
	}
}

/**
 * This is the default action which informs the API user they've made an unsupported request
 */
class RequestNotAllowedAction implements Action {

	public function process() {
		// nothing to do
	}

	public function generateResponse() {
		err('request not allowed');
		//send 405 Response
		//header(); 
	}
}

err($_SERVER);
err($_GET);
err($_POST);
err($_ENV);

$app = new App($_SERVER, $_GET, $_POST, $_ENV);
$app->setAllowedMethods(array(
	'/' 		=> array('GET'), 
	'/AuditLog' => array('POST')
));
$app->setActionMap(array(
	'/' 		=> 'ApiUsageAction', 
	'/AuditLog' => 'RecordAuditLogAction'
));
$app->begin();
