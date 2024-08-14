import 'package:flutter/material.dart';
import 'package:mqtt_client/mqtt_client.dart';
import 'package:mqtt_client/mqtt_server_client.dart';

void main() {
  runApp(MyApp());
}

enum MesState {
  GOOD,
  SUCCESS,
  ERROR,
  TOOLONG,
  WAIT,
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
  String _inputMessage = '';
  bool _isTextVisible = false;

  final String _broker = 'mqtt.cetools.org';
  final int _port = 1884;
  final String _clientIdentifier = 'myClient';
  final String _username = 'student';
  final String _password = 'ce2021-mqtt-forget-whale';
  final String _topic1 = 'student/ucfnuax/GesStatus';
  final String _publishTopic = 'student/ucfnuax/Message'; // 新增的发布主题

  MesState currentState = MesState.GOOD;

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
          if(pt == "0"){
            currentState = MesState.WAIT;
          }
          else if( pt == "1"){
            currentState = MesState.SUCCESS;
          }
          setState(() {
          });
        }
      });
    });
  }

  void _onConnected() {
    setState(() {
      _connectionStatus = 'Connected';
    });
    _client!.subscribe(_topic1 , MqttQos.atMostOnce);
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

  void validateAndTransformMessage(String message) {
    final RegExp regExp = RegExp(r'^[a-zA-Z\s]+$');
    if (regExp.hasMatch(message)) {
      currentState = MesState.GOOD;
    } else if (message.length >= 80) {
      currentState = MesState.TOOLONG;
    } else {
      currentState = MesState.ERROR;
    }
  }

  void _publishMessage() {
    if (_client!.connectionStatus!.state == MqttConnectionState.connected) {
      validateAndTransformMessage(_inputMessage);
      if (currentState == MesState.GOOD) {
        String message = _inputMessage.toUpperCase();
        final builder = MqttClientPayloadBuilder();
        builder.addString(message);
        _client!.publishMessage(
            _publishTopic, MqttQos.atMostOnce, builder.payload!);
      }
    } else {
      print('MQTT is not connected');
    }
    setState(() {
      _isTextVisible = true; // 显示隐藏的文本
    });
  }

  @override
  void dispose() {
    _disconnect();
    super.dispose();
  }

  Text message() {
    if (_client!.connectionStatus!.state == MqttConnectionState.connected) {
      return Text('CONNECTED',
          style: TextStyle(fontSize: 30, color: Colors.green));
    } else {
      return Text('DISCONNECTED',
          style: TextStyle(fontSize: 30, color: Colors.red));
    }
  }

  Text messageState(){
    String message= "";
    Color color = Colors.red;

    if(currentState == MesState.GOOD){
      message = "Sending";
      color = Colors.yellow;
    }
    else if(currentState == MesState.SUCCESS){
      message = "SUCCESS";
      color = Colors.green;
    }
    else if(currentState == MesState.TOOLONG){
      message = "Message Too Long";
      color = Colors.red;
    }
    else if(currentState == MesState.WAIT){
      message = "Send Message Later\nToo Many Messsage";
      color = Colors.red;
    }
    else if(currentState == MesState.ERROR){
      message = "Invalid Content\nOnly Alphabets and Space";
      color = Colors.red;
    }

    return Text(
        message,
        style: TextStyle(
        color: color,
        fontSize: 15,
    ));
  }


  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text('Lumiverse'),
      ),
      body: Padding(
        padding: const EdgeInsets.fromLTRB(16.0,0,16,0),
        child: SingleChildScrollView(child:Column(
          children: [
            SizedBox(height: 100),
            Container(
              padding: EdgeInsets.symmetric(horizontal: 10.0, vertical: 5.0),
              decoration: BoxDecoration(
                  border: Border.all(color: Colors.grey, width: 2.0),
                  borderRadius: BorderRadius.circular(10.0),
                  color: Colors.white),
              child: message(),
            ),
            SizedBox(height: 150),
            Row(
              mainAxisAlignment: MainAxisAlignment.center, // 将输入框和按钮居中对齐
              children: [
                Expanded(
                  child: TextField(
                    onChanged: (value) {
                      _inputMessage = value;
                    },
                    decoration: InputDecoration(
                      hintText: 'Enter message',
                      border: OutlineInputBorder(
                        borderRadius: BorderRadius.circular(30.0),
                      ),
                    ),
                  ),
                ),

              ],
            ),
            SizedBox(height: 10),
            Visibility(
              visible: _isTextVisible,
              child: messageState(),
              ),
            SizedBox(height: 20),
            ElevatedButton(
              onPressed: _publishMessage,
              child: Text('Send'),
            ),

          ],
        ),
      ),),
    );
  }
}
