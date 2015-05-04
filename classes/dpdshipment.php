<?php

class DpdShipment
{
	/**
	 * Path to ParcelShopFinder webservice wsdl.
	 */
	CONST WEBSERVICE_SHIPMENT = 'ShipmentService/V3_2/?wsdl';
	
	public $url;
	public $login;
	public $request;
	
	public $result;
	
	public function __construct(DpdLogin $login)	
	{
		$this->login = $login;
		$this->url = $this->getWebserviceUrl($login->url);
	}
	
	public function send()
	{
		if(!isset($request['printOptions']))
		{
			$this->request['printOptions']['printerLanguage'] = 'PDF';
			$this->request['printOptions']['paperFormat'] = 'A6';
		}
		
		$counter = 0;
		$stop = false;
		while(!$stop
			&& $counter < 3)
		{
			try {
				$client = new SoapClient($this->url, array('trace' => 1));
				
				$soapHeader = $this->login->getSoapHeader();
				$client->__setSoapHeaders($soapHeader);

				$result = $client->storeOrders($this->request);
			} 
			catch (SoapFault $soapE) 
			{
				switch($soapE->getCode())
				{
					case 'soap:Server':
						$splitMessage = explode(':', $soapE->getMessage());
						switch($splitMessage[0])
						{
							case 'cvc-complex-type.2.4.a':
								$newMessage = 'One of the mandatory fields is missing.';
								break;
							case 'cvc-minLength-valid':
								$newMessage = 'One of the values you provided is not long enough.';
								break;
							case 'cvc-maxLength-valid':
								$newMessage = 'One of the values you provided is too long.';
								break;
							case 'Fault occured':
								if($soapE->detail && $soapE->detail->authenticationFault)
								{
									$counter++;
									if($counter < 3)
									{	
										switch($soapE->detail->authenticationFault->errorCode)
										{
											case 'LOGIN_5':
												$this->login->refresh();
												continue 4;
												break;
											case 'LOGIN_6':
												$this->login->refresh();
												continue 4;
												break;
											default:
												$newMessage = $soapE->detail->authenticationFault->errorMessage;
												break;
										}
									} 
									else
										$newMessage = 'Maximum retries exceeded: ' . $soapE->detail->authenticationFault->errorMessage;
								}
								else
									$newMessage = 'Something went wrong, please use the Exception trace to find out. ';
								break;
							default:
								$newMessage = $soapE->getMessage() . $client->__getLastRequest();
								break;
						}
						break;
					case 'soap:Client':
						switch($soapE->getMessage())
						{
							case 'Error reading XMLStreamReader.':
								$newMessage = 'It looks like their is a typo in the xml call.';
								break;
							default:
								$newMessage = $soapE->getMessage();
								break;
						}
						break;
					default:
						$newMessage = $soapE->getMessage() . $client->__getLastRequest();
						break;
				}
				throw new Exception($newMessage, $soapE->getCode(), $soapE);
			} 
			catch (Exception $e) 
			{
				throw new Exception('Something went wrong with the connection to the DPD server', $e->getCode(), $e);
			}
			$stop = true;
		}
		
		if(isset($result->orderResult->shipmentResponses->faults))
		{
		
			$fault = $result->orderResult->shipmentResponses->faults;
			$message = $fault->message .' ('. $fault->faultCode . ')';
			
			throw new Exception($message);
		}
		
		$this->result = $result;
		return $result;
	}
	
	/**
	* Add trailing slash to url if not exists.
	*
	* @param $url
	* @return mixed|string
	*/
	protected function getWebserviceUrl($url)
	{
			if (substr($url, -1) != '/') {
					$url = $url . '/';
			}

			return $url . self::WEBSERVICE_SHIPMENT;
	}
}