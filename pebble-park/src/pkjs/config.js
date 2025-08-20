module.exports = [
    {
      "type": "heading",
      "defaultValue": "Pebble Park Configuration"
    },
    {
        "type": "section",
        "items": [
            {
                "type": "input",
                "messageKey": "Username",
                "defaultValue": "",
                "label": "Username",
                "description": "8 characters max",
                "attributes": {
                    "type": "text"
                }
            },
            {
                "type": "input",
                "messageKey": "Password",
                "defaultValue": "",
                "label": "Password",
                "description": "20 characters max",
                "attributes": {
                    "type": "text"
                }
            },
            {
                "type": "input",
                "messageKey": "Email",
                "defaultValue": "",
                "label": "This email will only be used to recover your account. 40 characters max",
                "attributes": {
                    "type": "text"
                }
            },
            {
                "type": "input",
                "messageKey": "Status",
                "defaultValue": "",
                "label": "Share your status with everyone! 20 characters max",
                "attributes": {
                    "type": "text"
                }
            }
        ]
    },
    {
        "type": "submit",
        "defaultValue": "Submit"
    }
  ];
  