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
				"messageKey": "COLOUR_HOUR",
				"label": "Hour",
				"defaultValue": "FFAA00",
				"sunlight": true,
				"capabilities": ["NOT_BW"],
			},
			{
				"type": "color",
				"messageKey": "COLOUR_MINUTE",
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
				"messageKey": "COLOUR_DATE",
				"label": "Date",
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
		]
	},
	{
		"type": "submit",
		"defaultValue": "Save"
	},
	{
		"type": "text",
		"defaultValue": " Version 0.13",
	},
];


