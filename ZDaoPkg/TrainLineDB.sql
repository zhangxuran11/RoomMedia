DROP TABLE IF EXISTS tb_lines_info;
CREATE TABLE tb_lines_info (
          train_id INTEGER PRIMARY KEY  AUTOINCREMENT,
          line_name VARCHAR2(50) NOT NULL UNIQUE
	);

DROP TABLE IF EXISTS tb_station_info;
CREATE TABLE tb_station_info (
          station_id  INTEGER,
		  indexNu	INTEGER	,
          train_id   INTEGER REFERENCES tb_lines_info(train_id) ON UPDATE CASCADE ON DELETE CASCADE,
          station_name_en VARCHAR2(50),
          station_name_th VARCHAR2(50),
          start_time VARCHAR2(10),
          arrive_time VARCHAR2(10),
          lat   REAL ,
          lng   REAL ,
		  sea_level REAL ,
		  CONSTRAINT pk_tb_station_info PRIMARY KEY (station_id,train_id)
	);

DROP TABLE IF EXISTS tb_railway_info;
CREATE TABLE tb_railway_info (
          sample_id INTEGER   NOT NULL,
          train_id   INTEGER REFERENCES tb_lines_info(train_id) ON UPDATE CASCADE ON DELETE CASCADE,
		  station_id INTEGER NOT NULL,
          lng REAL NOT NULL,
          lat REAL NOT NULL,
          sea_level REAL NOT NULL,
		  CONSTRAINT pk_tb_railway_info PRIMARY KEY (sample_id,train_id)
	);

insert into tb_lines_info (train_id,line_name) values (23,'testline');
insert into tb_lines_info (train_id,line_name) values (30,'testline2');

insert into tb_station_info (station_id,train_id,indexNu,station_name_en,station_name_th,start_time,arrive_time,lng,lat,sea_level) 
	values (1,23,1,'Babg Sue','บางชื่อ','12:04','14:08',99.448809,-20.392249,11);
insert into tb_station_info (station_id,train_id,indexNu,station_name_en,station_name_th,start_time,arrive_time,lng,lat,sea_level) 
	values (2,23,2,'Bang Khen','บางเขน','14:12','15:00',100.241736,-17.239439,14.1);
insert into tb_station_info (station_id,train_id,indexNu,station_name_en,station_name_th,start_time,arrive_time,lng,lat,sea_level) 
	values (3,23,3,'Slak si','หลักสี่','15:04','16:32',102.066840,-16.892855,11.4);
insert into tb_station_info (station_id,train_id,indexNu,station_name_en,station_name_th,start_time,arrive_time,lng,lat,sea_level) 
	values (4,23,4,'Don Muang','ดอนเมือง','16:39','17:55',100.028876,-16.189873,15.8);
insert into tb_station_info (station_id,train_id,indexNu,station_name_en,station_name_th,start_time,arrive_time,lng,lat,sea_level) 
	values (5,23,5,'Slak Huk','หลักหก','18:02','18:35',105.044134,-14.925137,8.2);
insert into tb_station_info (station_id,train_id,indexNu,station_name_en,station_name_th,start_time,arrive_time,lng,lat,sea_level) 
	values (6,23,6,'Khlong Rang Sit','คลองรังสิต','18:41','20:08',100.523238,-13.755243,7.1);
insert into tb_station_info (station_id,train_id,indexNu,station_name_en,station_name_th,start_time,arrive_time,lng,lat,sea_level) 
	values (7,23,7,'Rang Sit','รังสิต','20:14','21:34',99.146090,-11.034510,-4.2);
insert into tb_station_info (station_id,train_id,indexNu,station_name_en,station_name_th,start_time,arrive_time,lng,lat,sea_level) 
	values (8,23,8,'Khlong Nueng','คลองหนึ่ง','21:38','22:03',99.732801,-7.858479,0.2);
insert into tb_station_info (station_id,train_id,indexNu,station_name_en,station_name_th,start_time,arrive_time,lng,lat,sea_level) 
	values (9,30,1,'Babg Sue2','บางชื่อ2','12:04','14:08',99.448809,-20.392249,11);
insert into tb_station_info (station_id,train_id,indexNu,station_name_en,station_name_th,start_time,arrive_time,lng,lat,sea_level) 
	values (10,30,2,'Bang Khen2','บางเขน2','14:12','15:00',100.241736,-17.239439,14.1);
insert into tb_station_info (station_id,train_id,indexNu,station_name_en,station_name_th,start_time,arrive_time,lng,lat,sea_level) 
	values (4,30,3,'Slak si2','หลักสี่2','15:04','16:32',102.066840,-16.892855,11.4);
insert into tb_station_info (station_id,train_id,indexNu,station_name_en,station_name_th,start_time,arrive_time,lng,lat,sea_level) 
	values (11,30,4,'Don Muang2','ดอนเมือง2','16:39','17:55',100.028876,-16.189873,15.8);
insert into tb_station_info (station_id,train_id,indexNu,station_name_en,station_name_th,start_time,arrive_time,lng,lat,sea_level) 
	values (8,30,5,'Slak Huk2','หลักหก2','18:02','18:35',105.044134,-14.925137,8.2);
insert into tb_station_info (station_id,train_id,indexNu,station_name_en,station_name_th,start_time,arrive_time,lng,lat,sea_level) 
	values (12,30,6,'Khlong Rang Sit2','คลองรังสิต2','18:41','20:08',100.523238,-13.755243,7.1);
insert into tb_station_info (station_id,train_id,indexNu,station_name_en,station_name_th,start_time,arrive_time,lng,lat,sea_level) 
	values (13,30,7,'Rang Sit2','รังสิต2','20:14','21:34',99.146090,-11.034510,-4.2);
insert into tb_station_info (station_id,train_id,indexNu,station_name_en,station_name_th,start_time,arrive_time,lng,lat,sea_level) 
	values (14,30,8,'Khlong Nueng2','คลองหนึ่ง2','21:38','22:03',99.732801,-7.858479,0.2);
