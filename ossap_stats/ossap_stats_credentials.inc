<?php

/**
 * @file of type inc for credentials
 * Statistics for OpenScholar Single Access Point (OSSAP).
 */

/**
* ossap_stats_check_credentials()
* authenticate credentials by creating a new Google_client object and 
* sending it to google analytics
* ossap_stats_check_credentials
* no parameters
* creates the google_client object
* returns the validated service object
*/

function ossap_stats_check_credentials(){
  //declare and set the variables 
  $output ='';
  $access_type = 'offline_access';
  $scope = 'https://www.googleapis.com/auth/analytics.readonly';
  $service_gmail = variable_get('ossap_stats_service_gmail');
  $client_id = variable_get('ossap_stats_client_id');
  $fname = variable_get('ossap_stats_private_key_file_name');
  $fpath = variable_get('ossap_stats_private_key_path_to_file');
  $fpkey = $fpath.'/'.$fname.'';

  //create a new instance of Google_client
  $client = new Google_Client();
  // set the application name
  $client->setApplicationName('OSSAP Stats');   

  //create the credentials object to be sent to google analytics
  $client->setAssertionCredentials(new Google_AssertionCredentials($service_gmail, array($scope), 
                                   file_get_contents($fpkey)));                              
  $client->setClientId($client_id);
  $client->setAccessType($access_type);
  $client->getAccessToken();

  //create a new instance of google_analyticsservice and send the credentials object
  $service = new Google_AnalyticsService($client);
    
  //return the object returned by google analytics
  return $service;    
}
    
/**
* ossap_stats_design_output
* parameters = $type which could be a pageview or visits
* taps into google analytics and retrieves the results for 
* most frequent page views and page visited
* returns the pageviews and visit object
*/

function ossap_stats_design_output($type){
 //declare and set the variables 
  $output ='';
  $app_name = 'OSSAP Statistics';
  $metrics_pageviews = 'ga:pageviews';
  $sort_pageviews = '-ga:pageviews';
  $analytics_id = variable_get('ossap_stats_analytics_id');
   
  //set start and end dates for day, week and month
  //start and end dates for day are same. For week start date is 8 days before
  //likewise for month it is 31 days earlier. The end date
  //remains same as end date for day
  $start_date = $end_date = date('Y-m-d', strtotime('- 1 day'));
  $weeklyStartDate = date('Y-m-d', strtotime('- 8 day'));
  $monthlyStartDate = date('Y-m-d', strtotime('- 31 day'));
  
  //get credentials 
  $service = ossap_stats_check_credentials();
  
  //max_results is set to 28
  $max_results = 28;

  //set values depending on the type of the parameter
  if ($type == 'sites'){
    $dimensions = 'ga:hostname';
  }
  else if ($type == 'pages') {
    $dimensions = 'ga:hostname,ga:pagePath';
  }
  
  //set optional parameters like sort, dimensions etc
  $optparams = array ('dimensions' => $dimensions,'sort' => $sort_pageviews,'max-results' => $max_results);
  
  try {
    //get results from the service object for all three intervals
    $dailyResults = $service->data_ga->get($analytics_id, $start_date, $end_date, $metrics_pageviews, $optparams);
    $weeklyResults = $service->data_ga->get($analytics_id, $weeklyStartDate, $end_date, $metrics_pageviews, $optparams);
    $monthlyResults = $service->data_ga->get($analytics_id, $monthlyStartDate, $end_date, $metrics_pageviews, $optparams);
	
  } catch (Exception $e) {
    watchdog(WATCHDOG_ERROR, "Unable to get data from Google Data, {$analytics_id}, {$start_date}, {$end_date}, {$metrics}, {$optparams}");
    return array('rows' => array());
  }
  
  //add the results to $results array with interval as key
  $results = array(
   'day' => $dailyResults,
   'week' => $weeklyResults,
   'month' => $monthlyResults  
  );
  //return the results
  return $results;
}

/**
* ossap_stats_return_users()
* no parameters 
* taps into google analytics and retrieves the results for 
* users that are currently active
*/
function ossap_stats_return_users(){
  //declare variables
  $dimensions = 'rt:source';
  $rt_users = 'rt:activeUsers';
  $analytics_id = variable_get('ossap_stats_analytics_id');

  //validate the credentials
  $service = ossap_stats_check_credentials();
  //set the optional parameters
  $optparams = array ('dimensions' => $dimensions);

  //get result 
  $results = $service->data_realtime->get($analytics_id, $rt_users);

  //return the results
  return $results;
}