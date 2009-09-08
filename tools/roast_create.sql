create table warehouse
(
	id int not null primary key identity(1,1)
,	name varchar(64) not null unique
,	type varchar(64) default 'logical_warehouse'
,	comments varchar(128)
,	created datetime default getdate()
);

create table sku
(
	id int not null primary key identity(1,1)
,	name varchar(128) not null
,	is_kit bit default 0
,	type varchar(64) default 'product'
,	country varchar(64)
,	created datetime default getdate()
);
-- drop table sku;
create table kit_detail
(
	id int not null primary key identity(1,1)
,	master_sku_id int not null references sku(id)
,	detail_sku_id int not null references sku(id)
,	qty int not null
,	created datetime default getdate()
);

create table loads
(
	id int not null primary key identity(1,1)
,	sku_id int not null references sku(id)
,	qty int default 0
,	wh_id int not null references warehouse(id)
,	change_date datetime default getdate()
,	created datetime default getdate()
)

create table sku_change
(
	id int not null primary key identity(1,1)
,	sku_id int not null references sku(id)
,	delta_qty int not null
,	wh_id int not null references warehouse(id)
,	dest_wh_id int not null references warehouse(id)
,	group_id int
,	type_id	varchar(64) not null
,	load_id int not null references loads(id)
,	calc_qty int
,	load_qty int
,	created datetime default getdate()
)

create table worker
(
	id int not null primary key identity(0,1)
,	name varchar(32) unique
,	full_name varchar(256)
,	type varchar(32) default 'roaster'
,	disabled bit default 0
,	created datetime default getdate()
)
drop table worker;

insert into worker(name, full_name) values('root', 'root of all evil');
select * from worker;

create table roast
(
	id int not null primary key identity(1,1)
,	start_date datetime
,	end_date datetime
,	load_date datetime
,	unload_date datetime
,	input_sku_id int not null references sku(id)
,	input_qty int not null
,	output_sku_id int not null references sku(id)
,	output_qty int not null
,	status varchar(32)
,	worker_id int references worker(id)
,	input_wh_id int references warehouse(id)
,	roast_wh_id int not null references warehouse(id)
,	output_wh_id int references warehouse(id)
,	sku_change_id int references sku_change(id)
,	max_input_temp int
,	max_inner_temp int
,	created datetime default getdate()
)

create table roast_detail
(
	id int not null primary key identity(1,1)
,	roast_id int not null references roast(id)
,	row_date datetime not null
,	input_temp int not null -- входная температура (колорифер)
,	inner_temp int not null -- внутренняя температура (барабан)
,       outer_temp int not null -- температура помещения
)

-- Таблица задает на какой склад попадает товар после обжарки
-- на конкретном складе
create table roast_sku_moveaway
(
	id int not null primary key identity(1,1)
,	sku_id int not null references sku(id) -- какой товар получили после обжарки
,	roast_wh_id int not null references warehouse(id) -- где жарили
,	dest_wh_id int not null references warehouse(id) --склад выгрузки послеобжарки (куда положить)
,	created datetime default getdate()
)