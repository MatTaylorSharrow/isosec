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
 * Take an array and add a new array of values, creating the new values array 
 * if necessary
 *
 * Just little helper function to tidy up code
 * 
 * @param array By reference
 */
function array_append_or_create(&$array, $key, $value) {
	if (isset($array[$key]) && is_array($array[$key])) {
		$array[$key][] = $value;
	} else {
		$array[$key] = array($value);
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

		if ( ! $this->config) {
			echo 'Could not load config file. Aborting';
			return;
		}

		$action = $this->createAction();

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
		$conn = new mysqli($conf['host'], $conf['user'], $conf['password'], $conf['database']);

		if ($conn->connect_error) {
			return false;
		}

		$this->conn = $conn;
		return true;
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
	private $valid_input = array();

	// we need a validation rule for each input
	private $input_validation_rules = array(
		'customer' => '/^[A-Za-z]{1,3}$/',
		'product' => '/^[A-Za-z]{1,3}$/',
		'event_timestamp' => '/^[0-9:\- ]*$/',
		'device_id' => '/^[A-Za-z0-9]{16,32}$/',
	);

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
	 * Good boys and girls always validate their input. 
	 *
	 * If you want to use data within the application you have to provide a 
	 * validation rule. We use preg_match here for speed of development, though 
	 * this can be quite slow and doesn't allow for bespoke error messages 
	 * without futher work.  In a real system I'd look to replace these with 
	 * functions defined on the data items within the domain.
	 * 
	 * @return array errors keyed on input field name with value an array of 
	 *               error messages pertaining to that input field
	 */
	private function validateInput() {
		$errors = array(); 

		$input_vars = $this->app->getRawRequestInput();

		foreach ($input_vars as $var_name => $var_value) {

			// check if a validation rules exists for the input field
			if ( ! isset($this->input_validation_rules[$var_name]) ) {
				array_append_or_create($errors, $var_name, 'This input field does not have a validation rules and so can not be accepted.');
				continue;
			}

			// validate variable name
			if ( ! preg_match('/^[A-Za-z0-9_]*$/', $var_name)) {
				// hmmm what shall we do.  This wouldn't be the users fault, it would be 
				// poor client programming or malicious input so telling the user would 
				// be somewhat pointless as they can't do anything to fix it.

				array_append_or_create($errors,  $var_name, 'Invalid data input field.');
				continue;
			}

			// validate the input fields actual value
			if ( ! preg_match($this->input_validation_rules[$var_name], trim($var_value))) {
				array_append_or_create($errors, $var_name, 'Does not match the criteria for valid input.');
				continue;
			}

			$this->valid_input[$var_name] = $var_value;
		}

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

		if ( ! $this->app->hasDbConnection() ) {
			return;
		}

		$conn = $this->app->getDbConn();

		if ($conn) {
			$stmt = $conn->prepare("INSERT INTO AuditLog (customer, product, raised, received) VALUES(? , ? , ? , NOW()) ");
			if ( ! $stmt) {
				err('preparing statement failed');
				return;
			}

			$stmt->bind_param(
				"sss", 
				$this->valid_input['customer'], 
				$this->valid_input['product'], 
				$this->valid_input['event_timestamp']
//				,  $this->valid_input['device_id']
			);

			$stmt->execute();

			$this->success = true;
		}
	}

	/**
	 * Inform the user to the result of the requested action.
	 *
	 */
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
