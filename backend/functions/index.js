const admin = require('firebase-admin');
admin.initializeApp();

// Export Recommendation AI Engine functions
const recommendation = require('./recommendation');
exports.getIrrigationRecommendation = recommendation.getIrrigationRecommendation;
exports.predictMotorFailure = recommendation.predictMotorFailure;

const functions = require('firebase-functions');

/**
 * Trigger Push Notifications on Critical Fault Audit Logs
 */
exports.onAuditLogCreated = functions.firestore
    .document('devices/{deviceId}/audit_logs/{logId}')
    .onCreate(async (snapshot, context) => {
        const deviceId = context.params.deviceId;
        const logData = snapshot.data();

        // Check if log is a critical fault trip
        if (logData.category === 'FAULT') {
            const db = admin.firestore();
            
            // Get device details
            const deviceDoc = await db.collection('devices').doc(deviceId).get();
            if (!deviceDoc.exists) return null;
            const deviceName = deviceDoc.data().name || "Pump Controller";
            const ownerId = deviceDoc.data().owner_id;

            // Get owner's FCM registration token
            const userDoc = await db.collection('users').doc(ownerId).get();
            if (!userDoc.exists) return null;
            const fcmToken = userDoc.data().fcm_token;

            if (fcmToken) {
                const payload = {
                    notification: {
                        title: `⚠️ MOTOR TRIP: ${deviceName}`,
                        body: `Fault: ${logData.event_name}. Reason: ${logData.description}`
                    },
                    data: {
                        deviceId: deviceId,
                        event_type: "FAULT_TRIP",
                        fault_name: logData.event_name
                    }
                };

                try {
                    await admin.messaging().send({
                        token: fcmToken,
                        notification: payload.notification,
                        data: payload.data
                    });
                    console.log(`Push notification sent for fault: ${logData.event_name}`);
                } catch (error) {
                    console.error("FCM Send failed", error);
                }
            }
        }
        return null;
    });
