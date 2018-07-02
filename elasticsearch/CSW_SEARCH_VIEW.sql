SELECT 
  "CSW_VISION_ID"."PARTY_ROWID" AS "PARTY ROW ID", 
  "CSW_VISION_ID"."VISION_ID" AS "VISION ID", 
  "CSW_PARTY_ADDR"."GBID" AS "GBID", 
  "CSW_PARTY_ADDR"."FIRST_NM" AS "FIRST NAME", 
  "CSW_PARTY_ADDR"."LAST_NM" AS "LAST NAME", 
  "CSW_PARTY_ADDR"."MID_NM" AS "MIDDLE NAME", 
  "CSW_PARTY_ADDR"."GENERATION_CD" AS "GENERATION CODE", 
  "CSW_PARTY_ADDR"."ADDR_LINE1" AS "ADDRESS", 
  "CSW_PARTY_ADDR"."CITY_NM" AS "CITY", 
  "CSW_PARTY_ADDR"."ST_CD" AS "STATE", 
  "CSW_PARTY_ADDR"."PSTL_CD" AS "ZIP", 
  "CSW_PLCY_NUM"."PLCY_NUM" AS "POLICY NUMBER", 
  "CSW_EMAIL"."EMAIL" AS "EMAIL", 
  MAX("CSW_PHONE"."PHONE_NUM") AS "PHONE", 
  MAX("CSW_VISION_ID"."LAST_UPDATE_DATE") AS "LAST UPDATE DATE"
FROM 
  public."CSW_VISION_ID", 
  public."CSW_PLCY_NUM", 
  public."CSW_PHONE", 
  public."CSW_PARTY_ADDR", 
  public."CSW_EMAIL"
WHERE 
  "CSW_PLCY_NUM"."PARTY_ROWID" = "CSW_VISION_ID"."PARTY_ROWID" AND
  "CSW_PHONE"."PARTY_ROWID" = "CSW_VISION_ID"."PARTY_ROWID" AND
  "CSW_PARTY_ADDR"."PARTY_ROWID" = "CSW_VISION_ID"."PARTY_ROWID" AND
  "CSW_EMAIL"."PARTY_ROWID" = "CSW_VISION_ID"."PARTY_ROWID"
GROUP BY 1,2,3,4,5,6,7,8,9,10,11,12,13

