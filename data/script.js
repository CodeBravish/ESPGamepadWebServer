// WEBSOCKET ==============
var gateway = `ws://${window.location.hostname}:5500/ws`
// var gateway = window.location.hostname == "localhost" ? `ws://192.168.0.5/ws` : `ws://192.168.0.3:5500/ws`
var websocket

window.addEventListener("load", (e) => {
    WebSocketInit()
})

function WebSocketInit() {
    console.log("Attempting to Establish a WebSocket Connection...")
    websocket = new WebSocket(gateway)
    dataHTML.innerHTML += gateway + "<br/>"
    websocket.onopen = onOpen
    websocket.onclose = onClose
    websocket.onmessage = onMessage
}

function onOpen(e) {
    console.log("Connection Opened")
    console.log(e)
}
function onClose(e) {
    console.log("Connection Closed")
    setTimeout(WebSocketInit, 2000)
}
function onMessage(e) {
    console.log(e.data)
    let dataObject = e.data
    dataHTML.innerHTML += dataObject + "<br/>"
}
function sendLoop() {
    websocket.send("Input," + xAxisValue +","+ throttleValue + ",")
}

// GAMEPAD ================
const gpConnectionState = document.getElementById("gp-connection-state")
const throttle = document.getElementById("throttle")
const xAxis = document.getElementById("x-axis")
const dataHTML = document.getElementById("data")
const TRIGGER_ID = 7
const POLLING_TIME_MS = 10
const DEAD_ZONE = 0.1
let xAxisValue = 0
let throttleValue = 0

function gamepadLoop(pollTime, gamepadIndex) {
    setInterval(() => {
        gp = navigator.getGamepads()[gamepadIndex]

        throttleValue = gp.buttons[TRIGGER_ID].value
        throttle.textContent = throttleValue

        if (gp.axes[0] > DEAD_ZONE || gp.axes[0] < -1 * DEAD_ZONE) {
            xAxisValue = gp.axes[0]
            xAxis.textContent = xAxisValue
        } else {
            xAxisValue = 0
            xAxis.textContent = xAxisValue

        }
        sendLoop()
    }, pollTime)
    sendLoop(100)
}

window.addEventListener("gamepadconnected", (e) => {
    gpConnectionState.textContent = "Connected 🟢"
    gamepadLoop(POLLING_TIME_MS, e.gamepad.index)
    console.log(e.gamepad)
})

window.addEventListener("gamepaddisconnected", (e) => {
    gpConnectionState.textContent = "Disconnected ❌"
})
