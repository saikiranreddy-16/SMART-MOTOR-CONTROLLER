const functions = require('firebase-functions');
const admin = require('firebase-admin');
const axios = require('axios');

/**
 * AI Engine: Calculate daily irrigation recommendations
 * Triggered daily or on-demand via HTTP
 */
exports.getIrrigationRecommendation = functions.https.onCall(async (data, context) => {
    // 1. Ensure user is authenticated
    if (!context.auth) {
        throw new functions.https.HttpsError('unauthenticated', 'User must login first.');
    }

    const deviceId = data.deviceId;
    if (!deviceId) {
        throw new functions.https.HttpsError('invalid-argument', 'Device ID is required.');
    }

    const db = admin.firestore();

    // 2. Fetch Device configuration and HP size
    const deviceDoc = await db.collection('devices').doc(deviceId).get();
    if (!deviceDoc.exists) {
        throw new functions.https.HttpsError('not-found', 'Device not found.');
    }
    const deviceData = deviceDoc.data();
    const pumpHp = deviceData.pump_hp || 5.0; // default 5HP pump

    // 3. Retrieve recent telemetry logs (last 24 hours)
    const telemetrySnapshot = await db.collection('devices').doc(deviceId)
        .collection('telemetry_history')
        .orderBy('hour_timestamp', 'desc')
        .limit(2) // Get last 2 hours block
        .get();

    let currentSoilMoisture = 40.0; // default baseline
    let currentTemp = 30.0;

    if (!telemetrySnapshot.empty) {
        const lastBlock = telemetrySnapshot.docs[0].data();
        if (lastBlock.readings && lastBlock.readings.length > 0) {
            const latestReading = lastBlock.readings[lastBlock.readings.length - 1];
            currentSoilMoisture = latestReading.s || currentSoilMoisture;
            currentTemp = latestReading.c || currentTemp;
        }
    }

    // 4. Query weather forecast from Open-Meteo API
    let rainProbability = 0;
    let forecastTemp = 32.0;

    try {
        const weatherResponse = await axios.get(
            `https://api.open-meteo.com/v1/forecast?latitude=17.3850&longitude=78.4867&hourly=precipitation_probability,temperature_2m&forecast_days=1`
        );
        const hourlyProb = weatherResponse.data.hourly.precipitation_probability;
        const hourlyTemp = weatherResponse.data.hourly.temperature_2m;
        
        // Take average of next 12 hours
        rainProbability = hourlyProb.slice(0, 12).reduce((a, b) => a + b, 0) / 12;
        forecastTemp = hourlyTemp.slice(0, 12).reduce((a, b) => a + b, 0) / 12;
    } catch (error) {
        console.error("Failed to fetch weather forecast, using default constants", error);
    }

    // 5. Run AI Decision Logic
    let recommendedDuration = 0;
    let reason = "";

    if (rainProbability > 65.0) {
        recommendedDuration = 0;
        reason = `Rain forecast is high (${rainProbability.toFixed(0)}%). Irrigation skipped to conserve water.`;
    } else if (currentSoilMoisture >= 60.0) {
        recommendedDuration = 0;
        reason = `Soil moisture is sufficient at ${currentSoilMoisture.toFixed(0)}%. No irrigation required.`;
    } else {
        // Compute deficits: target moisture is 55%
        const moistureDeficit = 55.0 - currentSoilMoisture;
        
        // Multiplier scaling: 5HP pump needs ~3 minutes per deficit percentage.
        // Inversely proportional to pump capacity (HP)
        const scaleFactor = 15.0 / pumpHp; // 5HP -> multiplier 3
        recommendedDuration = Math.round(moistureDeficit * scaleFactor);

        // Adjust for high temperature evaporation
        if (forecastTemp > 38.0) {
            recommendedDuration = Math.round(recommendedDuration * 1.2); // +20% run time
            reason = `Soil moisture is low (${currentSoilMoisture.toFixed(0)}%). Adjusting +20% for heatwave (${forecastTemp.toFixed(1)}C).`;
        } else {
            reason = `Soil moisture is low (${currentSoilMoisture.toFixed(0)}%). Water required to reach nominal 55% saturation.`;
        }

        // Cap irrigation at 120 minutes per cycle to avoid waterlogging/overheating
        if (recommendedDuration > 120) {
            recommendedDuration = 120;
            reason += " Capped at maximum 120 mins.";
        }
    }

    // 6. Write recommendation to database
    const recRef = db.collection('devices').doc(deviceId).collection('irrigation_recommendations').doc();
    const recommendation = {
        recommendation_id: recRef.id,
        timestamp: admin.firestore.FieldValue.serverTimestamp(),
        recommended_duration_minutes: recommendedDuration,
        rain_probability: rainProbability,
        soil_moisture_level: currentSoilMoisture,
        reason: reason,
        applied: false
    };
    await recRef.set(recommendation);

    return recommendation;
});

/**
 * Predict Motor Failures based on current imbalances & dry run history
 */
exports.predictMotorFailure = functions.pubsub.schedule('every 24 hours').onRun(async (context) => {
    const db = admin.firestore();
    const devicesSnapshot = await db.collection('devices').get();

    for (const doc of devicesSnapshot.docs) {
        const deviceId = doc.id;
        
        // Get recent safety trip history (last 7 days)
        const logsSnapshot = await db.collection('devices').doc(deviceId)
            .collection('audit_logs')
            .where('category', '==', 'FAULT')
            .where('timestamp', '>', new Date(Date.now() - 7 * 24 * 60 * 60 * 1000))
            .get();

        let dryRunCount = 0;
        let overcurrentCount = 0;

        logsSnapshot.forEach(logDoc => {
            const data = logDoc.data();
            if (data.event_name === 'DRY_RUN') dryRunCount++;
            if (data.event_name === 'OVER_CURRENT') overcurrentCount++;
        });

        // Failure threshold alerts
        if (dryRunCount >= 5) {
            await triggerNotification(deviceId, "Borewell Water Depleted", "System detected 5+ dry run trips in a week. Water level is critical.");
        }
        if (overcurrentCount >= 3) {
            await triggerNotification(deviceId, "Motor Mechanical Wear Detected", "Frequent overcurrent trips indicate possible bearing damage or impeller jam. Maintenance recommended.");
        }
    }
});

async function triggerNotification(deviceId, title, body) {
    const db = admin.firestore();
    const deviceDoc = await db.collection('devices').doc(deviceId).get();
    const ownerId = deviceDoc.data().owner_id;

    const userDoc = await db.collection('users').doc(ownerId).get();
    const fcmToken = userDoc.data().fcm_token;

    if (fcmToken) {
        const message = {
            notification: { title, body },
            token: fcmToken
        };
        await admin.messaging().send(message);
        console.log(`Alert sent to owner of ${deviceId}`);
    }
}
