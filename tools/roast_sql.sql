insert into sku (name) values('India Plantation A (Индия)');
insert into sku (name) values('India Plantation B (Индия)');
insert into sku (name) values('Смесь 1 Ароматизированная');
select * from sku;

insert into kit_detail(master_sku_id, detail_sku_id, qty) values(3,1,2);
insert into kit_detail(master_sku_id, detail_sku_id, qty) values(3,2,3);
select * from kit_detail;

select * from kit_detail join sku sku1 join sku sku2 on sku1.id=kit_detail.master_sku_id
on sku2.id=kit_detail.detail_sku_id;

select * from kit_detail join sku sku1 on sku1.id=kit_detail.master_sku_id

insert into warehouse(name) values('Склад сырья');
insert into loads (sku_id, qty, wh_id) values (1, 100, 1);

select * from loads;
select loads.id, sku.name, warehouse.name, loads.qty from loads, sku, warehouse
where loads.sku_id=sku.id and loads.wh_id=warehouse.id;

select loads.id, sku.name, warehouse.name, loads.qty 
from loads join sku 
on loads.sku_id=sku.id, loads join warehouse 
on loads.wh_id=warehouse.id;


commit;