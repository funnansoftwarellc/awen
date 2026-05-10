import QtQuick
import QtQuick.Window

Window {
    width: 1280
    height: 720
    visible: true
    title: "Awen"

    Canvas {
        anchors.centerIn: parent
        width: parent.width
        height: parent.height

        onPaint: {
            var ctx = getContext("2d")
            var centerX = width / 2
            var centerY = height / 2
            var radius = height * 0.4
            var startAngle = (30 - 90) * Math.PI / 180
            var endAngle = (30 + 10 - 90) * Math.PI / 180

            ctx.strokeStyle = "black"
            ctx.lineWidth = 2
            ctx.lineCap = "round"

            ctx.beginPath()
            ctx.arc(centerX, centerY, radius, startAngle, endAngle + Math.PI * 2 - 20 * Math.PI / 180, false)
            ctx.stroke()
        }
    }
}
