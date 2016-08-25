module.exports = [
	{ 
    	"type": "heading", 
    	"defaultValue": "Simply Analog",
		"size": 1,
	},
	{
		"type": "section",
		"items": [
			{
				"type": "heading",
				"defaultValue": "Colour Selection"
			},
			{
				"type": "color",
				"messageKey": "COLOUR_BACKGROUND",
				"label": "Background",
				"defaultValue": "000000",
				"sunlight": true
			},
			{
				"type": "color",
				"messageKey": "COLOUR_HAND_HOUR",
				"label": "Hour",
				"defaultValue": "FFAA00",
				"sunlight": true,
				"capabilities": ["NOT_BW"],
			},
			{
				"type": "color",
				"messageKey": "COLOUR_HAND_MINUTE",
				"label": "Minute",
				"defaultValue": "FFFFFF",
				"sunlight": true,
				"capabilities": ["NOT_BW"],
			},
			{
				"type": "color",
				"messageKey": "COLOUR_WEEKDAY",
				"label": "Weekday",
				"defaultValue": "FFFFFF",
				"sunlight": true,
				"capabilities": ["NOT_BW"],
			},
			{
				"type": "color",
				"messageKey": "COLOUR_DAY",
				"label": "Day",
				"defaultValue": "FFFFFF",
				"sunlight": true,
				"capabilities": ["NOT_BW"],
			},
			{
				"type": "color",
				"messageKey": "COLOUR_MONTH",
				"label": "Month",
				"defaultValue": "FFFFFF",
				"sunlight": true,
				"capabilities": ["NOT_BW"],
			},
// 			{
// 				"type": "color",
// 				"messageKey": "COLOUR_BLUETOOTH",
// 				"label": "Blutooth Disconnection",
// 				"defaultValue": "FF0000",
// 				"sunlight": true,
// 				"capabilities": ["NOT_BW"],
// 			}
		]
	},
// 	{
// 		"type": "section",
// 		"items": [
// 			{
// 				"type": "heading",
// 				"defaultValue": "Battery"
// 			},
// 			{
// 				"type": "toggle",
// 				"messageKey": "TOGGLE_BATTERY",
// 				"label": "Show battery icon",
// 				"defaultValue": false,
// 			},
// 			{
// 				"type": "select",
// 				"messageKey": "SELECT_BATTERY_PERCENT",
// 				"defaultValue": "20",
// 				"label": "Low battery percent",
// 				"options": [
// 					{ 
// 						"label": "10%",
// 						"value": "10" 
// 					},
// 					{ 
// 						"label": "20%",
// 						"value": "20" 
// 					},
// 					{ 
// 						"label": "30%",
// 						"value": "30" 
// 					},
// 					{ 
// 						"label": "40%",
// 						"value": "40" 
// 					},
// 					{ 
// 						"label": "50%",
// 						"value": "50" 
// 					},
// 					{ 
// 						"label": "60%",
// 						"value": "60" 
// 					},
// 				]
// 			},
// 		]
//	},
// 	{
// 		"type": "section",
// 		"items": [
// 			{
// 				"type": "heading",
// 				"defaultValue": "Custom Text"
// 			},
// 			{
// 				"type": "checkboxgroup",
// 				"messageKey": "CHECK_DATE",
// 				"label": "Select events to be displayed:",
// 				"defaultValue": [false, false, false],
// 				"options": ["New Years Day", "Halloween", "Christmas Day"]
// 			}
// 		]
// 	},
	{
		"type": "submit",
		"defaultValue": "Save"
	}
];

