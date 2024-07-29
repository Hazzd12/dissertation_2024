import 'package:flutter/material.dart';
import 'package:mqtt_client/mqtt_client.dart';
import 'package:mqtt_client/mqtt_server_client.dart';

void main() {
  runApp(MyApp());
}

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: MqttPage(),
    );
  }
}

class MqttPage extends StatefulWidget {
  @override
  _MqttPageState createState() => _MqttPageState();
}

class _MqttPageState extends State<MqttPage> {
  MqttServerClient? _client;
  String _connectionStatus = 'Disconnected';
  String _topic1Message = 'No data';
  String _topic2Message = 'No data';

  final String _broker = 'mqtt.cetools.org';
  final int _port = 1884;
  final String _clientIdentifier = 'myClient';
  final String _username = 'student';
  final String _password = 'ce2021-mqtt-forget-whale';
  final String _topic1 = 'UCL/OPS/Garden/WST/dvp2/outHumidity';
  final String _topic2 = 'UCL/OPS/Garden/WST/dvp2/barometer_mbar';

  @override
  void initState() {
    super.initState();
    _connect();
  }

  Future<void> _connect() async {
    _client = MqttServerClient(_broker, _clientIdentifier)
      ..port = _port
      ..logging(on: true)
      ..keepAlivePeriod = 20
      ..onDisconnected = _onDisconnected
      ..onConnected = _onConnected
      ..onSubscribed = _onSubscribed;

    final connMess = MqttConnectMessage()
        .authenticateAs(_username, _password)
        .withClientIdentifier(_clientIdentifier)
        .startClean()
        .withWillQos(MqttQos.atMostOnce);
    _client!.connectionMessage = connMess;

    try {
      await _client!.connect();
    } catch (e) {
      print('Exception: $e');
      _disconnect();
    }

    _client!.updates!.listen((List<MqttReceivedMessage<MqttMessage>> c) {
      final MqttPublishMessage recMess = c[0].payload as MqttPublishMessage;
      final String pt =
      MqttPublishPayload.bytesToStringAsString(recMess.payload.message);

      setState(() {
        if (c[0].topic == _topic1) {
          _topic1Message = pt;
        } else if (c[0].topic == _topic2) {
          _topic2Message = pt;
        }
      });
    });
  }

  void _onConnected() {
    setState(() {
      _connectionStatus = 'Connected';
    });
    _client!.subscribe(_topic1, MqttQos.atMostOnce);
    _client!.subscribe(_topic2, MqttQos.atMostOnce);
  }

  void _onDisconnected() {
    setState(() {
      _connectionStatus = 'Disconnected';
    });
  }

  void _onSubscribed(String topic) {
    print('Subscribed to $topic');
  }

  void _disconnect() {
    _client!.disconnect();
    _onDisconnected();
  }

  @override
  void dispose() {
    _disconnect();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text('MQTT Client'),
      ),
      body: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          children: [
            Text('Connection Status: $_connectionStatus'),
            SizedBox(height: 20),
            Text('Topic 1: $_topic1'),
            Text('Message: $_topic1Message'),
            SizedBox(height: 20),
            Text('Topic 2: $_topic2'),
            Text('Message: $_topic2Message'),
          ],
        ),
      ),
    );
  }
}
