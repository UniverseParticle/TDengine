package com.taos.example;

import com.taosdata.jdbc.TSDBDriver;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.util.Properties;

public class RESTConnectExample {
    // ANCHOR: main
    public static void main(String[] args) throws SQLException {
        String jdbcUrl = "jdbc:TAOS-RS://localhost:6041?user=root&password=taosdata";
        Properties connProps = new Properties();
        connProps.setProperty(TSDBDriver.PROPERTY_KEY_TIME_ZONE, "UTC-8");
        Connection conn = DriverManager.getConnection(jdbcUrl, connProps);
        System.out.println("Connected");
        conn.close();
    }
    // ANCHOR_END: main
}