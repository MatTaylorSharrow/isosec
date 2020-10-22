<?php

// Rest Entities
// - / - list of available entities and actions
//   /AuditLog - POST

/* request json
{
	"customer": "string",
	"product": "string", 
	"event_timestamp": "datetime", 
	"device_id": "string"
}
*/

// response codes
// 200 - All OK
//

/* success response json
{
}
*/

/* error response json 
{

}
*/

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

	public function __construct($server, $get, $post, $env) {
		$this->server = $server;
		$this->get = $get;
		$this->post = $post;
		$this->env = $env;
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
		$action = $this->createAction();
		$action->process();
		$action->generateResponse();
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
				$actionClass = $this->action_map[$this->server['REQUEST_URI']];
				$action = new $actionClass($this);
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

	public function process() {
err('processing audit log');
		// nothing to do
	}

	public function generateResponse() {
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
