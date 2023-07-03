import requests
import json
headersTest = {'Content-type': 'application/json', 'X-Goog-Api-Key':'AIzaSyBUdJXMeCrOQZyBVrKHxI_kFj5uY1f8uIs','X-Goog-FieldMask':'routes.duration,routes.distanceMeters,routes.polyline.encodedPolyline'}
r = requests.post('https://routes.googleapis.com/directions/v2:computeRoutes', json='{"origin": {"location": {"latLng": {"latitude": 37.419734,"longitude": -122.0827784}}},"destination": {"location": {"latLng": {"latitude": 37.417670,"longitude": -122.079595}}},"travelMode": "DRIVE","routingPreference": "TRAFFIC_AWARE","departureTime": "2023-10-15T15:01:23.045123456Z","computeAlternativeRoutes": false,"routeModifiers": {"avoidTolls": false,"avoidHighways": false,"avoidFerries": false},"languageCode": "en-US","units": "IMPERIAL"}', headers=headersTest)
r = requests.post('https://routes.googleapis.com/directions/v2:computeRoutes', json=json.load(open('payload.json')), headers=headersTest)
print(f"Status Code: {r.status_code}, Response: {r.json()}")
result = r.json()
print(result['routes'][0]['duration'])