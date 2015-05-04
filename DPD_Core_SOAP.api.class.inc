<?php
/**
 * DPD SOAP API library/class
 * Interaction with DPD SOAP Web services.
 */
 

include_once dirname(__FILE__).'/classes/dpdlogin.php';
include_once dirname(__FILE__).'/classes/dpdparcelshopfinder.php';
include_once dirname(__FILE__).'/classes/dpdshippingmethods.php';

class DPD_Core_SOAP {

   private $Url; // URL for SOAP service.
   private $Language; // Message language.
   private $DelisID; // Your DelisID.
   private $Password; // Your password.


  /**
   * Constructor which initializes the consumer.
   *
   * @param string $delisID
   *   Your DelisID
   * @param string $password
   *   Your password
   */
  public function __construct($DelisID, $Password, $Language = 'en_EN', $LiveServer = true) {
    $this->Url = $LiveServer ? 'https://public-ws.dpd.com/services/' : 'https://public-ws-stage.dpd.com/services/';
    $this->DelisID = $DelisID;
    $this->Password = $Password;
    $this->Language = $Language;
  }

  /**
   * Make a request to the DPD services.
   *
   * @param $data
   * @return array;
   */
  public function MakeRequest($data = array()) {
		$response = array('a');
		
    return $response;
  }

  /**
   * setOrder method REST request to a DPD service for multiple orders data.
   * Generates a DPD parcel label as Base64-String and starts a shipping order.
   *
   * @param $action - Enumeration of different order start types.
   * Possible:
   * . startOrder = 0 (Order start)
   * . checkOrderData = 1 (Order check only)
   * @param $settings - Settings for order start
   * @param $orders_data - Array contains orders data with their shipping address information,
   * parcel information, ParcelShop ID.
   * example:
		$order_data = array(
      'OrderAction' => $action,
      'OrderSettings' => array(
				'ShipDate' => date('m.d.Y'),
				'LabelSize' => 'PDF_A4',
				'LabelStartPosition' => 'UpperLeft',
			),
     'OrderDataList' => array(
        // First order
        array(
         'ShipAddress' => array(
           'Company' => '',
           'Salutation' => '',
           'Name' => '',
           'Street' => '',
           'HouseNo' => '',
           'Country' => '',
           'ZipCode' => '',
           'City' => '',
           'State' => '',
           'Phone' => '',
           'Mail' => '',
          ),
         'ParcelShopID' => 0,
         'ParcelData' => array(
            'ShipService' => '',
            'Weight' => '',
            'Content' => '',
            'YourInternalID' => '',
            'Reference1' => '',
            'Reference2' => '',
            'COD' => array(
              'Purpose' => '',
              'Amount' => '',
              'Payment' => '',
            ),
          ),
        ),
        // Second order - same structure as for the first one
        // ...
     ),
    );
   * @return array;
   */
  public function setOrders($action, $settings = array(), $orders_data) {
		try	{
			$login = new DpdLogin($this->delisID, $this->password, $this->Url);
		}
		catch (Exception $e) {
			throw new Exception('Error occured: ' . $e->getMessage());
		}
		
		$shipment = new DpdShipment($login);
		
		foreach($orders_data['OrderDataList'] as $key => $order_data){
			$shipment->request['order'][$key] = array(
				'generalShipmentData' => array(
					'mpsCustomerReferenceNumber1' => $orders_data['ParcelData']['Reference1']
					,'mpsCustomerReferenceNumber2' => $orders_data['ParcelData']['Reference2']
					,'sendingDepot' => $login->depot
					,'product' => 'CL'
					,'sender' => array(
						'name1' => $orders_data['ShipperAddress']['Company']
						,'name2' => $orders_data['ShipperAddress']['Name']
						,'street' => $orders_data['ShipperAddress']['Street']
						,'HouseNo' => $orders_data['ShipperAddress']['HouseNo']
						,'country' => $orders_data['ShipperAddress']['Country']
						,'zipCode' => $orders_data['ShipperAddress']['ZipCode']
						,'city' => $orders_data['ShipperAddress']['City']
					)
					,'recipient' => array(
						'name1' => $orders_data['ConsigneeAddress']['Company']
						,'name2' => $orders_data['ConsigneeAddress']['Name']
						,'street' => $orders_data['ConsigneeAddress']['Street']
						,'HouseNo' => $orders_data['ConsigneeAddress']['HouseNo']
						,'country' => $orders_data['ConsigneeAddress']['Country']
						,'zipCode' => $orders_data['ConsigneeAddress']['ZipCode']
						,'city' => $orders_data['ConsigneeAddress']['City']
						,'phone' => $orders_data['ConsigneeAddress']['Phone']
						,'email' => $orders_data['ConsigneeAddress']['Mail']
					)
				)
			);
			
			$shipment->request['order']['parcels'] = array(
				'customerReferenceNumber1' => $orders_data['ParcelData']['Reference1']
				,'customerReferenceNumber2' => $orders_data['ParcelData']['Reference2']
				,'weight' => $orders_data['ParcelData']['Weight']
			);
			
			$shipment->request['order']['productAndServiceData']['orderType'] = 'consignment';
			
			// Predict
			if($orders_data['ParcelData']['ShipService'] == 1){
				if($orders_data['ConsigneeAddress']['Mail'])
					$shipment->request['order']['productAndServiceData']['predict'] = array(
						'channel' => '1'
						,'value' => $orders_data['ConsigneeAddress']['Mail']
						,'language' => 'EN'
					);
				elseif($orders_data['ConsigneeAddress']['Phone'])
					$shipment->request['order']['productAndServiceData']['predict'] = array(
						'channel' => '3'
						,'value' => $orders_data['ConsigneeAddress']['Phone']
						,'language' => 'EN'
					);
			}
			
			// ParcelShop
			if($orders_data['ParcelData']['ShipService'] == 4){
				if($orders_data['ConsigneeAddress']['Mail'])
					$shipment->request['order']['productAndServiceData']['parcelShopDelivery']['parcelShopNotification'] = array(
						'channel' => '1'
						,'value' => $orders_data['ConsigneeAddress']['Mail']
						,'language' => 'EN'
					);
				elseif($orders_data['ConsigneeAddress']['Phone'])
					$shipment->request['order']['productAndServiceData']['parcelShopDelivery']['parcelShopNotification'] = array(
						'channel' => '3'
						,'value' => $orders_data['ConsigneeAddress']['Phone']
						,'language' => 'EN'
					);
			}
		}
		
		$response;
		try {
			$response = $shipment->send();
		} 
		catch (Exception $e) {
			throw new Exception('Error occured: ' . $e->getMessage());
		}

    return $response;
  }

  /**
   * setOrder method REST request to a DPD service for single order data.
   * Generates a DPD parcel label as Base64-String and starts a shipping order.
   *
   * @param $action - Enumeration of different order start types.
   * @param $settings - Settings for order start
   * @param $ship_address - Array contains shipping address information.
   * @param $parcel_data - Array contains parcel information.
   * @param $parcel_shop_id - ParcelShop ID of the receiving ParcelShop. Relevant for shipping product "Shop_Delivery".
   * @return array;
   */
  public function setOrder($action, $settings = array(), $ship_address = array(), $parcel_data = array(), $parcel_shop_id = 0) {
		$settings += $this->defaultOrderSettings();
    $ship_address += $this->defaultShipAddress();
    $parcel_data += $this->defaultParcelData();
    $order_data = array(
      'OrderAction' => $action,
      'OrderSettings' => $settings,
      'OrderDataList' => array(
        array(
          'ShipAddress' => $ship_address,
          'ParcelShopID' => $parcel_shop_id,
          'ParcelData' => $parcel_data,
        ),
      ),
    );

    $response = $this->setOrders($action, $settings = array(), $order_data);

    return $response;
  }

  /**
   * Constructs default Order Settings array.
   *
   * @param $additional_params
   * @return array;
   */
  private function defaultOrderSettings() {
    $orderSettings = array(
      'ShipDate' => date('m.d.Y'), // Shipping date (Format: dd.mm.yyyy)
      // Note: No parcel pick-up on Sunday and public holidays.
      // You get a list of valid ship days for a zip code using "getZipCodeRules".
      'LabelSize' => 'PDF_A4', // Enumeration of different label size types.
      // Possible values:
      // . PDF_A4 = 0
      // . PDF_A6 = 1
      // . ZPL_A6 = 2
      'LabelStartPosition' => 'UpperLeft', //Enumeration of the different label printing positions.
      // Possible values:
      // . UpperLeft = 0
      // . UpperRight = 1
      // . LowerLeft = 2
      // . LowerRight = 3
    );

    return $orderSettings;
  }

  /**
   * Constructs default Order Settings array.
   *
   * @return array;
   */
  private function defaultShipAddress() {
    $ShipAddress = array(
      'Company' => '', // Company name
      'Salutation' => '', // Salutation (e.g. Mr., Mrs.)
      'Name' => '', // Name of the contact person
      'Street' => '', // Street name
      'HouseNo' => '', // House number
      'Country' => '', // Possible Values: Alpha3, Alpha2, ISO3166, Country name
      // Example: DEU, DE, 276, Deutschland
      'ZipCode' => '', // Zip code of a city
      'City' => '', // City name, if applicable city district
      // Example:
      // Aschaffenburg, Obernau
      'State' => '', // ISO3166-2 code of a state
      // Important:
      // If „USA“ or „CAN“ is selected as country, state is mandatory!
      // For all other countries, it is not allowed to specify a state!
      'Phone' => '', // Phone number (mobile, too.)
      // Notes concerning allowed characters:
      // . numbers 0-9
      // . any number of "blanks"
      // . "+" and "-"
      // . "(" and ")"
      'Mail' => '', // E-mail address (all known standard formats, valid according to general understatement.)

    );

    return $ShipAddress;
  }

  /**
   * Constructs default Parcel Data array.
   *
   * @return array;
   */
  private function defaultParcelData() {
    $ParcelData = array(
      'ShipService' => '0', // Enumeration of the DPD shipping products.
      // Possible values:
      // . Classic = 0
      // . Classic_Predict = 1
      // . Classic_COD = 2
      // . Classic_COD_Predict = 3
      // . Shop_Delivery = 4
      // . Shop_Return = 5
      // . Classic_Return = 6 (Only single order start!)
      'Weight' => '', // Parcel weight: 31,5 kg at maximum
      'Content' => '', // Parcel content description
      'YourInternalID' => '', // Internal reference field for linking the DPD parcel number with your internal system
      // (individual specification).
      'Reference1' => '', // Reference text 1 (individual specification)
      'Reference2' => '', // Reference text 2 (individual specification)
      // Example: DEU, DE, 276, Deutschland
      'COD' => array( // Contains "cash on delivery" information
        'Purpose' => '', // "Cash on delivery" Payment
        'Amount' => '', // "Cash on delivery" amount
        'Payment' => '', // Enumeration of payment type "COD"
        // Possible Values:
        // . Cash = 0 (Payment in cash)
        // . Cheque = 1 (Payment with cheque)
      ),
    );

    return $ParcelData;
  }

  /**
   * getParcelLifeCycle method REST request to a DPD service for a given parcel.
   * Returns the whole tracking data of a DPD Parcel by specifying the parcel number.
   *
   * @param $parcel_no - 14-digit parcel number (with preceding "0").
   * @return array;
   */
  public function getParcelLifeCycle($parcel_no) {
    $parcel_data = array(
      'ParcelNo' => $parcel_no,
    );

    $response = $this->MakeRequest('ParcelLifeCycle', $parcel_data);

    return $response;
  }

  /**
   * getParcelShopFinder method REST request to a DPD service.
   * Returns the master data of one ore more DPD ParcelShops
   * (100 simultaneously at maximum), as well as the unique ParcelShop identification number ("ParcelShopID").
   *
   * @param $search_data - an array with Address data or geo coordinatses.
   * @param $search_mode - the desired search mode type.
   * Possible values:
   * . SearchByAddress = 0 (Search via Shop address data)
   * . SearchByGeoData = 1 (Search via geo-coordinates).
   * @param $hide_on_closed_at - Fades out the ParcelShops, which are closed at a given date and time.
   * Format: yyyy-MM-dd HH:mm.
   * @param $search_geodata - an array with geo coordinates.
   * @param $need_service - Enumeration of the desired services, a ParcelShop offers.
   * Possible Values:
   * . Standard = 0
   * . ConsigneePickup = 1 (Pick-up by recipient)
   * . ReturnService = 2 (Return acceptance)
   * . ExpressService = 3 (Express shipping)
   * . PrepaidService = 4 (Parcel payment in advance)
   * . CashOnDeliveryService = 5 (Cash payment at delivery)
   * @param $max_results - Maximum amount of matches (100 simultaneously at maximum).
   * @return object response;
   */
  public function getParcelShopFinder($search_data, $search_mode = 0, $max_results = 100, $need_service = 0, $hide_on_closed_at = 'null') {
    if ($search_mode === 0) {
      $response = $this->getParcelShopFinderByAddress($search_data, $hide_on_closed_at, $need_service, $max_results = 100);
    }
    elseif($search_mode == 1) {
      $response = $this->getParcelShopFinderByGeoData($search_data['longitude'], $search_data['latitude'], $hide_on_closed_at, $need_service, $max_results = 100);
    }

    return $response;
  }

  /**
   * getParcelShopFinder by Address data.
   *
   * @param $search_address - an array with Address data or geo coordinates.
   * @return object response;
   */
  public function getParcelShopFinderByAddress($search_address, $max_results = 100, $need_service = 0, $hide_on_closed_at = 'null') {
    try	{
			$login = new DpdLogin($this->DelisID, $this->Password, $this->Url);
		}
		catch (Exception $e) {
			throw new Exception('Error occured: ' . $e->getMessage());
		}
		
		$parcelshopfinder;
		
		try	{
			$parcelshopfinder = new DpdParcelShopFinder($login);
		}
		catch (Exception $e) {
			throw new Exception('Error occured: ' . $e->getMessage());
		}

    $parcelshopfinder->search($search_address);
				
    return $this->mapParcelShopData($parcelshopfinder->results);
  }

  /**
   * getParcelShopFinder by geo coordinates.
   *
   * @param $search_geodata - an array with geo coordinates.
   * @return object response;
   */
  public function getParcelShopFinderByGeoData($longitute, $latitude, $max_results = 100, $need_service = 0, $hide_on_closed_at = 'null') {
		try	{
			$login = new DpdLogin($this->DelisID, $this->Password, $this->Url);
		}
		catch (Exception $e) {
			throw new Exception('Error occured: ' . $e->getMessage());
		}
		
		$parcelshopfinder;
		
		try	{
			$parcelshopfinder = new DpdParcelShopFinder($login);
		}
		catch (Exception $e) {
			throw new Exception('Error occured: ' . $e->getMessage());
		}

    $parcelshopfinder->search(array('long' => $longitute, 'lat' => $latitude));

    return $this->mapParcelShopData($parcelshopfinder->results);
  }
	
	private function mapParcelShopData($results){
		
		$result = new stdClass();
		$result->ParcelShopList = array();
		
		foreach($results as $parcelshopID => $data){
			$shop = new stdClass();
			
			$shop->ParcelShopID = $parcelshopID;
			
			$shop->ShopAddress = new stdClass();
			$shop->ShopAddress->Company = $data->company;
			$shop->ShopAddress->Salutation = '';
			$shop->ShopAddress->Name = '';
			$shop->ShopAddress->Street = $data->street;
			$shop->ShopAddress->HouseNo = $data->houseNo;
			$shop->ShopAddress->Country = $data->isoAlpha2;
			$shop->ShopAddress->ZipCode = $data->zipCode;
			$shop->ShopAddress->City = $data->city;
			$shop->ShopAddress->State = '';
			$shop->ShopAddress->Phone = $data->phone;
			$shop->ShopAddress->Mail = $data->email;
						
			$shop->Homepage = $data->homepage;
			
			$shop->GeoData = new stdClass();
			$shop->GeoData->Distance = $data->distance;
			$shop->GeoData->Longitude = $data->longitude;
			$shop->GeoData->Latitude = $data->latitude;
			$shop->GeoData->CoordinateX = $data->coordinateX;
			$shop->GeoData->CoordinateY = $data->coordinateY;
			$shop->GeoData->CoordinateZ = $data->coordinateZ;
			
			$shop->ShopServiceList = array('PickupByConsignee');
			
			$shop->OpeningHoursList = array();
			foreach($data->openingHours as $day){
				$newDay = new stdClass;
				
				$newDay->WeekDay = $day->weekday;
				
				$morning = new stdClass();
				$morning->TimeFrom = $day->openMorning;
				$morning->TimeEnd = $day->closeMorning;
				
				$afternoon = new stdClass();
				$afternoon->TimeFrom = $day->openAfternoon;
				$afternoon->TimeEnd = $day->closeAfternoon;
				
				$newDay->OpenTimeList = array($morning, $afternoon);
				
				$shop->OpeningHoursList[] = $newDay;
			}
			
			$shop->ExtraInfo = $data->extraInfo;
			
			$result->ParcelShopList[] = $shop;
		}
		
    return $result;
	}

  /**
   * Constructs default Search address data array.
   *
   * @return array;
   */
  public function defaultSearchAddress() {
    $SearchAddress = array(
      'Street' => '', // Street name
      'HouseNo' => '', // House number
      'ZipCode' => '', // Zip code of a city
      'City' => '', // City name
      'Country' => '', // Possible Values: Alpha3, Alpha2, ISO3166, Country name
      // Example: DEU, DE, 276, Deutschland
    );

    return $SearchAddress;
  }

  /**
   * getZipCodeRules method REST request to a DPD service.
   * Returns general shipping information for a pick-up point:
   * - Weekdays, on which no parcels can be picked up (public holidays, location-dependent conditions)
   *   Note: Saturday and Sunday are not pick-up days in general.
   *   So these days are not part of the list.
   * - Responsible pick-up depot
   * - Latest pick-up times for a parcel pick-up today
   *
   * The method does not require any specification of certain parameters.
   * @return object response;
   */
  public function getZipCodeRules() {
    $response = $this->MakeRequest('ZipCodeRules');

    return $response;
  }


}
