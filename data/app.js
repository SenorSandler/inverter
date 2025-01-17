var socket = new WebSocket("ws://192.168.4.1:1337");

// Function to receive data from ESP32 via websocket 
socket.onmessage = function (event) {
    var json_doc_str = event.data; // load JSON like String
    var json_doc = JSON.parse(json_doc_str); // parse String into a real JSON
    console.log(json_doc)

    // Input Voltage
    var input_voltage = json_doc["in_volt"];
    var input_voltage_label_text = document.getElementById("input_voltage_label");
    input_voltage_label_text.innerHTML = input_voltage;

    // Input Current
    var input_current = json_doc["in_amp"];
    var input_current_label_text = document.getElementById("input_current_label");
    input_current_label_text.innerHTML = input_current;

    var output_voltage = json_doc["out_volt"];
    var output_voltage_label_text = document.getElementById("output_voltage_label");
    output_voltage_label_text.innerHTML = output_voltage;

    var output_current = json_doc["out_amp"];
    var output_current_label_text = document.getElementById("output_current_label");
    output_current_label_text.innerHTML = output_current; 
}

// Function to send data to ESP32 via websocket
function sendDataToArduino(command, value) {
    var dataToSend = {
        command: command,
        value: value
    };
    var jsonData = JSON.stringify(dataToSend);  // Convert the data to JSON string
    socket.send(jsonData);  // Send the JSON data to Arduino
}

// Function to update Slider value
function updateSliderValue() {
    const slider = document.getElementById("stateSlider");
    const selectedStateLabel = document.getElementById("selectedState");

    const selectedState = getSelectedState(slider.value);
    selectedStateLabel.innerText = `${selectedState}`;

    // Send the selected state to Arduino
    sendDataToArduino("slider_state", selectedState);
}

function getSelectedState(value) {
    switch (parseInt(value)) {
        case 0:
            return "Off";
        case 1:
            return "Bipolar PWM On";
        case 2:
            return "Unipolar PWM On";
        default:
            return "Off";
    }
}

// Call the initialization code when the page loads
window.onload = function () {
    // Highcharts script
    var chartT = new Highcharts.Chart({
        chart: { renderTo: 'chart-temperature' },
        title: { text: 'Gemessene Werte' },
        series: [{
            showInLegend: false,
            data: [
                [new Date(2022, 0, 1, 12, 0).getTime(), 25.0],
                [new Date(2022, 0, 1, 12, 15).getTime(), 26.5],
                [new Date(2022, 0, 1, 12, 30).getTime(), 24.8],
                // Add more data points as needed
            ]
        }],
        plotOptions: {
            line: {
                animation: false,
                dataLabels: { enabled: true }
            },
            series: { color: '#059e8a' }
        },
        xAxis: {
            type: 'datetime',
            dateTimeLabelFormats: { second: '%H:%M:%S' }
        },
        yAxis: {
            title: { text: 'Spannung und Strom' }
        },
        credits: { enabled: false }
    });
};